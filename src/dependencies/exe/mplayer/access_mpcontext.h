#ifndef ACCESS_MPCONTEXT_H
#define ACCESS_MPCONTEXT_H

struct MPContext;
void *mpctx_get_video_out(struct MPContext *mpctx);
void *mpctx_get_audio_out(struct MPContext *mpctx);
void *mpctx_get_demuxer(struct MPContext *mpctx);
void *mpctx_get_playtree_iter(struct MPContext *mpctx);
void *mpctx_get_mixer(struct MPContext *mpctx);
int mpctx_get_global_sub_size(struct MPContext *mpctx);
int mpctx_get_osd_function(struct MPContext *mpctx);

#endif /* ACCESS_MPCONTEXT_H */
