/*
   (c) Copyright 2001-2008  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <config.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include <fusion/build.h>

#include <direct/debug.h>
#include <direct/messages.h>

#include <fusion/types.h>
#include <fusion/call.h>
#include <fusion/conf.h>

#include "fusion_internal.h"


D_DEBUG_DOMAIN( Fusion_Call, "Fusion/Call", "Fusion Call" );


#if FUSION_BUILD_MULTI

#if FUSION_BUILD_KERNEL

DirectResult
fusion_call_init (FusionCall        *call,
                  FusionCallHandler  handler,
                  void              *ctx,
                  const FusionWorld *world)
{
     FusionCallNew call_new;

     D_DEBUG_AT( Fusion_Call, "%s( %p, %p <%s>, %p, %p )\n", __FUNCTION__, call, handler,
                 direct_trace_lookup_symbol_at( handler ), ctx, world );

     D_ASSERT( call != NULL );
     D_ASSERT( handler != NULL );
     D_MAGIC_ASSERT( world, FusionWorld );
     D_MAGIC_ASSERT( world->shared, FusionWorldShared );

     /* Called from others. */
     call_new.handler = handler;
     call_new.ctx     = ctx;

     while (ioctl( world->fusion_fd, FUSION_CALL_NEW, &call_new )) {
          switch (errno) {
               case EINTR:
                    continue;
               default:
                    break;
          }

          D_PERROR ("FUSION_CALL_NEW");

          return DR_FAILURE;
     }

     memset( call, 0, sizeof(FusionCall) );

     /* Store handler, called directly when called by ourself. */
     call->handler = handler;
     call->ctx     = ctx;

     /* Store call and own fusion id. */
     call->call_id   = call_new.call_id;
     call->fusion_id = fusion_id( world );

     /* Keep back pointer to shared world data. */
     call->shared = world->shared;

     D_DEBUG_AT( Fusion_Call, "  -> call id %d\n", call->call_id );

     return DR_OK;
}

DirectResult
fusion_call_execute (FusionCall          *call,
                     FusionCallExecFlags  flags,
                     int                  call_arg,
                     void                *call_ptr,
                     int                 *ret_val)
{
     D_DEBUG_AT( Fusion_Call, "%s( %p, 0x%x, %d, %p )\n", __FUNCTION__, call, flags, call_arg, call_ptr );

     D_ASSERT( call != NULL );

     if (!call->handler)
          return DR_DESTROYED;

     D_DEBUG_AT( Fusion_Call, "  -> %s\n", direct_trace_lookup_symbol_at( call->handler ) );

     if (!(flags & FCEF_NODIRECT) && call->fusion_id == _fusion_id( call->shared )) {
          int                     ret;
          FusionCallHandlerResult result;

          result = call->handler( _fusion_id( call->shared ), call_arg, call_ptr, call->ctx, 0, &ret );

          if (result != FCHR_RETURN)
               D_WARN( "local call handler returned FCHR_RETAIN, need FCEF_NODIRECT" );

          if (ret_val)
               *ret_val = ret;
     }
     else {
          FusionCallExecute execute;

          execute.call_id  = call->call_id;
          execute.call_arg = call_arg;
          execute.call_ptr = call_ptr;
          execute.flags    = flags;

          while (ioctl( _fusion_fd( call->shared ), FUSION_CALL_EXECUTE, &execute )) {
               switch (errno) {
                    case EINTR:
                         continue;
                    case EINVAL:
//                         D_ERROR ("Fusion/Call: invalid call\n");
                         return DR_INVARG;
                    case EIDRM:
                         return DR_DESTROYED;
                    default:
                         break;
               }

               D_PERROR ("FUSION_CALL_EXECUTE");

               return DR_FAILURE;
          }

          if (ret_val)
               *ret_val = execute.ret_val;
     }

     return DR_OK;
}

DirectResult
fusion_call_return( FusionCall   *call,
                    unsigned int  serial,
                    int           val )
{
     FusionCallReturn call_ret;

     D_DEBUG_AT( Fusion_Call, "%s( %p, %u, %d )\n", __FUNCTION__, call, serial, val );

     D_ASSERT( call != NULL );

     D_DEBUG_AT( Fusion_Call, "  -> %s\n", direct_trace_lookup_symbol_at( call->handler ) );

     call_ret.call_id = call->call_id;
     call_ret.val     = val;
     call_ret.serial  = serial;

     while (ioctl (_fusion_fd( call->shared ), FUSION_CALL_RETURN, &call_ret)) {
          switch (errno) {
               case EINTR:
                    continue;
               case EIDRM:
                    D_WARN( "caller withdrawn (signal?)" );
                    return DR_NOCONTEXT;
               case EINVAL:
                    D_ERROR( "Fusion/Call: invalid call\n" );
                    return DR_DESTROYED;
               default:
                    break;
          }

          D_PERROR ("FUSION_CALL_RETURN");

          return DR_FAILURE;
     }

     return DR_OK;
}

DirectResult
fusion_call_destroy (FusionCall *call)
{
     D_DEBUG_AT( Fusion_Call, "%s( %p )\n", __FUNCTION__, call );

     D_ASSERT( call != NULL );
     D_ASSERT( call->handler != NULL );

     D_DEBUG_AT( Fusion_Call, "  -> %s\n", direct_trace_lookup_symbol_at( call->handler ) );

     while (ioctl (_fusion_fd( call->shared ), FUSION_CALL_DESTROY, &call->call_id)) {
          switch (errno) {
               case EINTR:
                    continue;
               case EINVAL:
                    D_ERROR ("Fusion/Call: invalid call\n");
                    return DR_DESTROYED;
               default:
                    break;
          }

          D_PERROR ("FUSION_CALL_DESTROY");

          return DR_FAILURE;
     }

     call->handler = NULL;

     return DR_OK;
}

void
_fusion_call_process( FusionWorld *world, int call_id, FusionCallMessage *msg )
{
     FusionCallHandler       call_handler;
     FusionCallReturn        call_ret;
     FusionCallHandlerResult result;

     D_DEBUG_AT( Fusion_Call, "%s()\n", __FUNCTION__ );

     D_MAGIC_ASSERT( world, FusionWorld );
     D_ASSERT( msg != NULL );

     call_handler = msg->handler;

     D_ASSERT( call_handler != NULL );

     D_DEBUG_AT( Fusion_Call, "  -> %s\n", direct_trace_lookup_symbol_at( call_handler ) );

     call_ret.call_id = call_id;
     call_ret.serial  = msg->serial;
     call_ret.val     = 0;

     result = call_handler( msg->caller, msg->call_arg, msg->call_ptr, msg->ctx, msg->serial, &call_ret.val );

     switch (result) {
          case FCHR_RETURN:
               while (ioctl (world->fusion_fd, FUSION_CALL_RETURN, &call_ret)) {
                    switch (errno) {
                         case EINTR:
                              continue;
                         case EIDRM:
                              D_WARN( "caller withdrawn (signal?)" );
                              return;
                         case EINVAL:
                              D_ERROR( "Fusion/Call: invalid call\n" );
                              return;
                         default:
                              D_PERROR( "FUSION_CALL_RETURN" );
                              return;
                    }
               }
               break;

          case FCHR_RETAIN:
               break;

          default:
               D_BUG( "unknown result %d from call handler", result );
     }
}

#else /* FUSION_BUILD_KERNEL */

#include <fcntl.h>
#include <unistd.h>


DirectResult
fusion_call_init (FusionCall        *call,
                  FusionCallHandler  handler,
                  void              *ctx,
                  const FusionWorld *world)
{
     D_ASSERT( call != NULL );
     D_ASSERT( handler != NULL );
     D_MAGIC_ASSERT( world, FusionWorld );
     D_MAGIC_ASSERT( world->shared, FusionWorldShared );

     memset( call, 0, sizeof(FusionCall) );

     call->call_id = ++world->shared->call_ids;

     /* Store handler, called directly when called by ourself. */
     call->handler = handler;
     call->ctx     = ctx;

     /* Store own fusion id. */
     call->fusion_id = fusion_id( world );

     /* Keep back pointer to shared world data. */
     call->shared = world->shared;

     return DR_OK;
}

DirectResult
fusion_call_execute (FusionCall          *call,
                     FusionCallExecFlags  flags,
                     int                  call_arg,
                     void                *call_ptr,
                     int                 *ret_val)
{
     DirectResult        ret = DR_OK;
     FusionWorld        *world;
     FusionCallMessage   msg;
     struct sockaddr_un  addr;
     
     D_ASSERT( call != NULL );

     if (!call->handler)
          return DR_DESTROYED;

     if (!(flags & FCEF_NODIRECT) && call->fusion_id == _fusion_id( call->shared )) {
          int                     ret;
          FusionCallHandlerResult result;

          result = call->handler( _fusion_id( call->shared ), call_arg, call_ptr, call->ctx, 0, &ret );

          if (result != FCHR_RETURN)
               D_WARN( "local call handler returned FCHR_RETAIN, need FCEF_NODIRECT" );

          if (ret_val)
               *ret_val = ret;
               
          return DR_OK;
     }
     
     world = _fusion_world( call->shared );
     
     msg.type     = FMT_CALL;  
     msg.caller   = world->fusion_id;
     msg.call_id  = call->call_id;
     msg.call_arg = call_arg;
     msg.call_ptr = call_ptr; 
     msg.handler  = call->handler;
     msg.ctx      = call->ctx;
     msg.flags    = flags;
     
     if (flags & FCEF_ONEWAY) {
          /* Invalidate serial. */
          msg.serial = -1;
          
          /* Send message. */
          addr.sun_family = AF_UNIX;
          snprintf( addr.sun_path, sizeof(addr.sun_path), 
                    "/tmp/.fusion-%d/%lx", call->shared->world_index, call->fusion_id );
         
          ret = _fusion_send_message( world->fusion_fd, &msg, sizeof(msg), &addr );
     }
     else {
          int       fd;
          socklen_t len;
          int       err;

          fd = socket( PF_LOCAL, SOCK_RAW, 0 );
          if (fd < 0) {
               D_PERROR( "Fusion/Call: Error creating local socket!\n" ) ;
               return DR_IO;
          }

          /* Set close-on-exec flag. */
          fcntl( fd, F_SETFD, FD_CLOEXEC );
          
          addr.sun_family = AF_UNIX;
          len = snprintf( addr.sun_path, sizeof(addr.sun_path), 
                          "/tmp/.fusion-%d/call.%x.", fusion_world_index( world ), call->call_id ); 
          
          /* Generate call serial (socket address is based on it). */
          for (msg.serial = 0; msg.serial <= 0xffffff; msg.serial++) {
               snprintf( addr.sun_path+len, sizeof(addr.sun_path)-len, "%x", msg.serial );
               err = bind( fd, (struct sockaddr*)&addr, sizeof(addr) );
               if (err == 0) {
                    chmod( addr.sun_path, 0660 );
                    /* Change group, if requested. */
                    if (fusion_config->shmfile_gid != (gid_t)-1)
                         chown( addr.sun_path, -1, fusion_config->shmfile_gid );
                    break;
               }
          }
          
          if (err < 0) {
               D_PERROR( "Fusion/Call: Error binding local socket!\n" );
               close( fd );
               return DR_IO;
          }

          /* Send message. */
          snprintf( addr.sun_path, sizeof(addr.sun_path), 
                    "/tmp/.fusion-%d/%lx", call->shared->world_index, call->fusion_id );
          
          ret = _fusion_send_message( fd, &msg, sizeof(msg), &addr );
          if (ret == DR_OK) {
               FusionCallReturn callret;
               /* Wait for reply. */
               ret = _fusion_recv_message( fd, &callret, sizeof(callret), NULL );
               if (ret == DR_OK) {
                    if (ret_val)
                         *ret_val = callret.val;
               } 
          }
          
          len = sizeof(addr);
          if (getsockname( fd, (struct sockaddr*)&addr, &len ) == 0)
               unlink( addr.sun_path );
          close( fd );
     }

     return ret;
}

DirectResult
fusion_call_return( FusionCall   *call,
                    unsigned int  serial,
                    int           val )
{
     struct sockaddr_un addr;
     FusionCallReturn   callret;
     
     D_ASSERT( call != NULL );

     addr.sun_family = AF_UNIX;
     snprintf( addr.sun_path, sizeof(addr.sun_path), 
               "/tmp/.fusion-%d/call.%x.%x", call->shared->world_index, call->call_id, serial );
               
     callret.type = FMT_CALLRET;
     callret.val  = val;
               
     return _fusion_send_message( _fusion_fd( call->shared ), &callret, sizeof(callret), &addr );
}

DirectResult
fusion_call_destroy (FusionCall *call)
{
     D_ASSERT( call != NULL );
     D_ASSERT( call->handler != NULL );

     call->handler = NULL;

     return DR_OK;
}

void
_fusion_call_process( FusionWorld *world, int call_id, FusionCallMessage *msg )
{
     FusionCallHandler       call_handler;
     FusionCallHandlerResult result;
     FusionCallReturn        callret;

     D_MAGIC_ASSERT( world, FusionWorld );
     D_ASSERT( msg != NULL );

     call_handler = msg->handler;

     D_ASSERT( call_handler != NULL );

     callret.type = FMT_CALLRET;
     callret.val  = 0;

     result = call_handler( msg->caller, msg->call_arg, msg->call_ptr, msg->ctx, msg->serial, &callret.val );
     switch (result) {
          case FCHR_RETURN:
               if (!(msg->flags & FCEF_ONEWAY)) {
                    struct sockaddr_un addr;

                    addr.sun_family = AF_UNIX;
                    snprintf( addr.sun_path, sizeof(addr.sun_path), 
                              "/tmp/.fusion-%d/call.%x.%x", fusion_world_index( world ), call_id, msg->serial );
               
                    if (_fusion_send_message( world->fusion_fd, &callret, sizeof(callret), &addr ))
                         D_ERROR( "Fusion/Call: Couldn't send call return (serial: 0x%08x)!\n", msg->serial );
               }
               break;

          case FCHR_RETAIN:
               break;

          default:
               D_BUG( "unknown result %d from call handler", result );
               break;
     }
}

#endif /* FUSION_BUILD_KERNEL */

#else  /* FUSION_BUILD_MULTI */

DirectResult
fusion_call_init (FusionCall        *call,
                  FusionCallHandler  handler,
                  void              *ctx,
                  const FusionWorld *world)
{
     D_ASSERT( call != NULL );
     D_ASSERT( call->handler == NULL );
     D_ASSERT( handler != NULL );

     /* Called locally. */
     call->handler = handler;
     call->ctx     = ctx;

     return DR_OK;
}

DirectResult
fusion_call_execute (FusionCall          *call,
                     FusionCallExecFlags  flags,
                     int                  call_arg,
                     void                *call_ptr,
                     int                 *ret_val)
{
     FusionCallHandlerResult ret;
     int                     val = 0;

     D_ASSERT( call != NULL );

     if (!call->handler)
          return DR_DESTROYED;

     ret = call->handler( 1, call_arg, call_ptr, call->ctx, 0, &val );
     if (ret != FCHR_RETURN)
          D_WARN( "only FCHR_RETURN supported in single app core at the moment" );

     if (ret_val)
          *ret_val = val;

     return DR_OK;
}

DirectResult
fusion_call_return( FusionCall   *call,
                    unsigned int  serial,
                    int           val )
{
     return DR_UNIMPLEMENTED;
}

DirectResult
fusion_call_destroy (FusionCall *call)
{
     D_ASSERT( call != NULL );
     D_ASSERT( call->handler != NULL );

     call->handler = NULL;

     return DR_OK;
}

#endif

