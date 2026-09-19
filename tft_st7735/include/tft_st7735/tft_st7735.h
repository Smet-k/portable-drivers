#ifndef TFT_ST7735_H
#define TFT_ST7735_H
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "tft_st7735_status.h"
#include "tft_st7735_port.h"

typedef enum {
    TFT_ST7735_COLOR_DEPTH_12BIT = 0,
    TFT_ST7735_COLOR_DEPTH_16BIT,
    TFT_ST7735_COLOR_DEPTH_18BIT
} tft_st7735_color_depth_t;

typedef enum {
    TFT_ST7735_ORIENTATION_PORTRAIT = 0,
    TFT_ST7735_ORIENTATION_LANDSCAPE,
    TFT_ST7735_ORIENTATION_PORTRAIT_INVERTED,
    TFT_ST7735_ORIENTATION_LANDSCAPE_INVERTED
} tft_st7735_orientation_t;

typedef struct {
    bool sleeping;
    bool display_on;
    bool normal;
    bool idle;
    bool inverted;
} tft_st7735_state_t;

typedef struct {
    uint8_t rtn;  // divider, 4 bits used
    uint8_t fp;   // front porch, 6 bits used
    uint8_t bp;   // back porch, 6 bits used
} tft_st7735_frame_rate_t;

typedef struct {
    tft_st7735_frame_rate_t normal;   // FRMCTR1
    tft_st7735_frame_rate_t idle;     // FRMCTR2
    struct {
        tft_st7735_frame_rate_t line;   // FRMCTR3, 1st-3rd params (line inversion)
        tft_st7735_frame_rate_t frame;  // FRMCTR3, 4th-6th params (frame inversion)
    } partial;
} tft_st7735_frame_rates_t;

typedef struct {
    uint8_t vrh;     // GVDD level, 5 bits used (PWCTR1 1st param)
    uint8_t ib_sel;  // AVDD current, 2 bits used (PWCTR1 2nd param)
} tft_st7735_pwctr1_t;

typedef struct {
    uint8_t bt;      // VGH/VGL level select, 3 bits used (PWCTR2 1st param)
} tft_st7735_pwctr2_t;

typedef struct {
    uint8_t ap;   // op-amp current, 3 bits used (PWCTR3/4/5)
    uint8_t dc;   // booster step-up cycle, 3 bits used
} tft_st7735_pwctr_mode_t;

typedef struct {
    uint8_t sapa; 
    uint8_t sapb; 
    uint8_t sapc; 
    uint8_t dcd;  
} tft_st7735_pwctr6_t;

typedef struct {
    tft_st7735_pwctr_mode_t normal;        // PWCTR3
    tft_st7735_pwctr_mode_t idle;          // PWCTR4
    tft_st7735_pwctr_mode_t partial;       // PWCTR5
    tft_st7735_pwctr6_t     partial_idle;  // PWCTR6
} tft_st7735_pwctr_modes_t;

typedef struct {
    tft_st7735_pwctr1_t pwctr1;
    tft_st7735_pwctr2_t pwctr2;
    tft_st7735_pwctr_modes_t pwctr_modes;
} tft_st7735_power_t;

// VMCTR1 is two parameters (VCOMH and VCOML levels), not a single byte.
typedef struct {
    uint8_t vmh; // 7 bits used, VCOMH level
    uint8_t vml; // 7 bits used, VCOML level
} tft_st7735_vmctr1_t;

typedef struct {
    tft_st7735_io_t io;
    uint16_t width;
    uint16_t height;
    uint8_t col_offset;
    uint8_t row_offset;
    tft_st7735_orientation_t orientation;
    tft_st7735_color_depth_t color_depth;
    tft_st7735_state_t state;

    tft_st7735_frame_rates_t frame_rate;
    tft_st7735_power_t power;
    tft_st7735_vmctr1_t vmctr1;
    uint8_t gamma_pos[16]; // GMCTRP1 -- raw table, treated as opaque bytes
    uint8_t gamma_neg[16]; // GMCTRN1 -- raw table, treated as opaque bytes
} tft_st7735_config_t;

typedef struct {
    tft_st7735_config_t config;
    bool initialized;
} tft_st7735_t;

// Returns a config pre-filled with RM reset-table defaults for every
// panel-tuning register (frame rate, power, VCOM). io, width, height,
// col_offset/row_offset, orientation, and color_depth are left zeroed --
// those are hardware-specific and must be set by the caller before calling
// tft_st7735_initialize(). gamma_pos/gamma_neg are also left zeroed: the RM
// defines no generic reset default for gamma, since it's tuned per physical
// panel by whoever built the glass, not by the driver IC itself.
tft_st7735_config_t tft_st7735_config_default(void);

/*STATE MACHINE*/
tft_st7735_status_t tft_st7735_set_sleep(tft_st7735_t* tft, bool sleep);
tft_st7735_status_t tft_st7735_set_display(tft_st7735_t* tft, bool on);
tft_st7735_status_t tft_st7735_set_normal(tft_st7735_t* tft, bool normal);
tft_st7735_status_t tft_st7735_set_idle(tft_st7735_t* tft, bool idle);
tft_st7735_status_t tft_st7735_set_invert(tft_st7735_t* tft, bool invert);

tft_st7735_status_t tft_st7735_set_frmctr_normal(tft_st7735_t* tft, tft_st7735_frame_rate_t rate);
tft_st7735_status_t tft_st7735_set_frmctr_idle(tft_st7735_t* tft, tft_st7735_frame_rate_t rate);
tft_st7735_status_t tft_st7735_set_frmctr_partial(tft_st7735_t* tft, tft_st7735_frame_rate_t line_inversion, tft_st7735_frame_rate_t frame_inversion);

tft_st7735_status_t tft_st7735_set_pwctr1(tft_st7735_t* tft, tft_st7735_pwctr1_t pwctr1);
tft_st7735_status_t tft_st7735_set_pwctr2(tft_st7735_t* tft, tft_st7735_pwctr2_t pwctr2);
tft_st7735_status_t tft_st7735_set_pwctr3(tft_st7735_t* tft, tft_st7735_pwctr_mode_t mode);
tft_st7735_status_t tft_st7735_set_pwctr4(tft_st7735_t* tft, tft_st7735_pwctr_mode_t mode);
tft_st7735_status_t tft_st7735_set_pwctr5(tft_st7735_t* tft, tft_st7735_pwctr_mode_t mode);
tft_st7735_status_t tft_st7735_set_pwctr6(tft_st7735_t* tft, tft_st7735_pwctr6_t mode);

tft_st7735_status_t tft_st7735_set_vmctr1(tft_st7735_t* tft, tft_st7735_vmctr1_t vmctr1);
tft_st7735_status_t tft_st7735_set_gamma(tft_st7735_t* tft, const uint8_t gamma_pos[16], const uint8_t gamma_neg[16]);

tft_st7735_status_t tft_st7735_set_madctl(tft_st7735_t* tft, tft_st7735_orientation_t orientation);
tft_st7735_status_t tft_st7735_reset(tft_st7735_t* tft);
tft_st7735_status_t tft_st7735_set_colmod(tft_st7735_t* tft, tft_st7735_color_depth_t depth);
tft_st7735_status_t tft_st7735_initialize(tft_st7735_t* tft, const tft_st7735_config_t* cfg);

tft_st7735_status_t tft_st7735_draw_rect(tft_st7735_t* tft, uint16_t x, uint16_t y,
                                          uint16_t w, uint16_t h, const uint8_t* pixels, size_t len);

#endif