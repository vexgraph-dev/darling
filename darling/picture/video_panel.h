#ifndef DARLING_VIDEO_PANEL_H
#define DARLING_VIDEO_PANEL_H

#include <stdbool.h>
#include <stdint.h>

#include "c23/constructor.h"
#include "darling/panel/panel.h"
#include "annotation/intention.h"

;;INTENTION("Video viewer node: Panel subclass with transport state (play/pause/seek over a VFS media path) and fit-never-stretch frame stamping. Mute v1: frames arrive from the decoder, never from callers.")

#define VIDEOPANEL_PATH_MAX 512

typedef struct VideoPanel {
    // --- Transport core (owner fields: playback state, not any part) ---
    Panel base;
    char path[VIDEOPANEL_PATH_MAX]; // Inline media path (VFS uri), NUL-terminated
    double durationSec;  // Total length, 0 = unknown yet
    double clockSec;     // Current presentation time
    bool playing;        // Transport state
    bool muted;          // v1 always true: no audio path yet
    float frameRate;     // Source fps, 0 = unknown
} VideoPanel;

VideoPanel *VideoPanel_0(void);
VideoPanel *VideoPanel_1(Panel *parent);

#define VideoPanel(...) CONSTRUCTOR_DISPATCH(VideoPanel, __VA_ARGS__)

void VideoPanel_setSourcePath(VideoPanel *v, const char *path);
const char *VideoPanel_getSourcePath(const VideoPanel *v);
void VideoPanel_clearSource(VideoPanel *v);

void VideoPanel_play(VideoPanel *v);
void VideoPanel_pause(VideoPanel *v);
void VideoPanel_toggle(VideoPanel *v);
void VideoPanel_seekTo(VideoPanel *v, double sec);

double VideoPanel_getDuration(const VideoPanel *v);
double VideoPanel_getClock(const VideoPanel *v);
bool VideoPanel_isPlaying(const VideoPanel *v);
bool VideoPanel_isMuted(const VideoPanel *v);
float VideoPanel_getFrameRate(const VideoPanel *v);

// Layout facade — inherit from Panel
static inline void VideoPanel_setLocation(VideoPanel *v, float x, float y)
    { if (v) Panel_setLocation(&(*v).base, x, y); }
static inline void VideoPanel_setSize(VideoPanel *v, float w, float h)
    { if (v) Panel_setSize(&(*v).base, w, h); }
static inline void VideoPanel_setAnchor(VideoPanel *v, int anchor)
    { if (v) Panel_setAnchor(&(*v).base, anchor); }
static inline void VideoPanel_setPivot(VideoPanel *v, int pivot)
    { if (v) Panel_setPivot(&(*v).base, pivot); }
static inline void VideoPanel_setBackgroundColor(VideoPanel *v, uint32_t color)
    { if (v) Panel_setBackgroundColor(&(*v).base, color); }

#endif
