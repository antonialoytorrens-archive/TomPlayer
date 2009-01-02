/**
 * \file viewmeter.c
 * \brief This module provides viewmeter creation and handling facilities
 *
 *
 * $URL$
 * $Rev$
 * $Author$
 * $Date$
 *
 */

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "label.h"
#include "viewmeter.h"

struct _vm_state{
    label_handle label;   /**< Underlyong label */
    char * format;        /**< Format of the view meter */    
    double inc;           /**< Value to increment / decrement */
    char buffer[64];      /**< buffer that holds the displayed string */
    double val;
    double min, max;
};

static void refresh(vm_handle hdl){
  snprintf(hdl->buffer, sizeof(hdl->buffer), hdl->format, hdl->val);
  hdl->buffer[sizeof(hdl->buffer)-1] = 0;
  label_set_text(hdl->label, hdl->buffer);
  return;
}

void vm_release(vm_handle hdl){
  if (hdl == NULL)  
    return;
  if (hdl->label){
    label_release(hdl->label);
    hdl->label = NULL;
  }
  if (hdl->format){
    free(hdl->format);
    hdl->format = NULL;
  }
  free(hdl);
}

vm_handle vm_create(const struct vm_config * config){
  struct label_config label_conf;
  vm_handle hdl;

  hdl = calloc(1, sizeof(*hdl));
  if (hdl == NULL){
    return NULL;
  }
  label_conf.dfb = config->dfb;
  label_conf.win = config->win;
  label_conf.pos = config->pos;
  label_conf.name = config->name;
  label_conf.font_color = config->font_color;
  label_conf.height = config->height;
  
  hdl->label = label_create(&label_conf);
  if (hdl->label == NULL){
    vm_release(hdl);
    return NULL;
  }

  hdl->format = strdup(config->format);
  hdl->inc = config->inc;
  hdl->min = config->min;
  hdl->max = config->max;
  return hdl;
}

bool vm_set_value(vm_handle hdl, double val){
  if ((val < hdl->min) ||
      (val > hdl->max)){
    return false;
  }
  hdl->val = val;
  refresh(hdl);
  return true;
}

bool vm_inc(vm_handle hdl){
  double tmp;
  tmp =  hdl->val  + hdl->inc;
  if (tmp > hdl->max){
    return false;
  }
  hdl->val = tmp;
  refresh(hdl);
  return true;
}

bool vm_dec(vm_handle hdl){
  double tmp;
  tmp =  hdl->val  - hdl->inc;
  if (tmp < hdl->min){
    return false;
  }
  hdl->val = tmp;
  refresh(hdl);
  return true;
}

double vm_get_value(vm_handle hdl){
  return hdl->val;
}

