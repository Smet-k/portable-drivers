#include "tft_st7735/tft_st7735.h"
#include <stdio.h>


#define SWRESET_CMD         0x01

/*Read CMDS, might be unnecessary, since i track a lot of information already.
only usecase i have in mind is a function to validate the current state, with
the data in struct*/
#define RDDID_CMD            0x04 
#define RDDST_CMD            0x09
#define RDDPM_CMD            0x0A
#define RDDCOLMOD_CMD        0x0C
#define RDDIM_CMD            0x0D
#define RDDSM_CMD            0x0E


#define SLEEP_IN_CMD         0x10
#define SLEEP_OUT_CMD        0x11
#define PARTIAL_MODE_CMD     0x12
#define NORMAL_MODE_CMD      0x13

#define INVERT_OFF_CMD       0x20
#define INVERT_ON_CMD        0x21
#define GAMSET_CMD           0x26
#define DISPLAY_OFF_CMD      0x28
#define DISPLAY_ON_CMD       0x29
#define CASET_CMD            0x2A
#define RASET_CMD            0x2B
#define RAMWR_CMD            0x2C

#define PARTIAL_AREA_SET_CMD 0x30
#define TEAR_EFFECT_OFF_CMD  0x34
#define TEAR_EFFECT_ON_CMD   0x35
#define MEMORY_ACCESS_CTRL_CMD 0x36
#define IDLE_OFF_MODE_CMD    0x38
#define IDLE_ON_MODE_CMD     0x39
#define COLMOD_SET_CMD       0x3A

#define FRMCTR1_CMD          0xB1
#define FRMCTR2_CMD          0xB2
#define FRMCTR3_CMD          0xB3
#define INVCTR_CMD           0xB4
#define DISSET5_CMD          0xB6

#define PWCTR1_CMD           0xC0
#define PWCTR2_CMD           0xC1
#define PWCTR3_CMD           0xC2
#define PWCTR4_CMD           0xC3
#define PWCTR5_CMD           0xC4
#define PWCTR6_CMD           0xFC

#define VMCTR1_CMD           0xC5
#define VMOFCTR_CMD          0xC7

#define WRID2_CMD            0xD1
#define WRID3_CMD            0xD2
#define NVFCTR1_CMD          0xD9
#define RDID1_CMD            0xDA
#define RDID2_CMD            0xDB
#define RDID3_CMD            0xDC
#define NVFCTR2_CMD          0xDE
#define NVFCTR3_CMD          0xDF

#define GMCTRP1_CMD          0xE0
#define GMCTRN1_CMD          0XE1

#define EXTCTRL_CMD          0xF0
#define VCOM4L_CMD           0xFF

static void write_cmd(tft_st7735_t* tft, uint8_t cmd) {
    tft->config.io.set_dc(false, tft->config.io.ctx);
    tft->config.io.spi_write(&cmd, 1, tft->config.io.ctx);
}

static void write_data(tft_st7735_t* tft, const uint8_t* data, size_t len) {
    tft->config.io.set_dc(true, tft->config.io.ctx);
    tft->config.io.spi_write(data, len, tft->config.io.ctx);
}

static void hw_reset(tft_st7735_t* tft){
    if(!tft) return; 
    tft->config.io.set_reset(0, tft->config.io.ctx);
    tft->config.io.delay_ms(10);  
    tft->config.io.set_reset(1, tft->config.io.ctx);
    tft->config.io.delay_ms(120); 

    tft->config.state = (tft_st7735_state_t){
        .sleeping = true, .display_on = false, .normal = true,
        .idle = false, .inverted = false,
    };
}

static void set_addr_window(tft_st7735_t* tft, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1){
    x0 += tft->config.col_offset;
    x1 += tft->config.col_offset;
    y0 += tft->config.row_offset;
    y1 += tft->config.row_offset;

    uint8_t caset[4] = {x0 >> 8, x0 & 0xFF, x1 >> 8, x1 & 0xFF};
    write_cmd(tft, CASET_CMD);
    write_data(tft, caset, 4);

    uint8_t raset[4] = {y0 >> 8, y0 & 0xFF, y1 >> 8, y1 & 0xFF};
    write_cmd(tft, RASET_CMD);
    write_data(tft, raset, 4);
}

tft_st7735_status_t tft_st7735_set_sleep(tft_st7735_t* tft, bool sleep){
    if(sleep) {
        write_cmd(tft, SLEEP_IN_CMD);
        tft->config.io.delay_ms(120);
    } else {
        write_cmd(tft, SLEEP_OUT_CMD);
        tft->config.io.delay_ms(120);
    }
    tft->config.state.sleeping = sleep;
    return TFT_ST7735_OK;
}

tft_st7735_status_t tft_st7735_set_display(tft_st7735_t* tft, bool on){
    if(on) {
        write_cmd(tft, DISPLAY_ON_CMD);
        tft->config.io.delay_ms(120);
    } else {
        write_cmd(tft, DISPLAY_OFF_CMD);
        tft->config.io.delay_ms(120);
    }
    tft->config.state.display_on = on;
    return TFT_ST7735_OK;
}

tft_st7735_status_t tft_st7735_set_normal(tft_st7735_t* tft, bool normal){
    if(normal) {
        write_cmd(tft, NORMAL_MODE_CMD);
    } else {
        if(tft->config.state.idle) {
                tft_st7735_set_idle(tft, false); // Exit Idle mode before entering Partial mode
        }
        write_cmd(tft, PARTIAL_MODE_CMD);
    }
    tft->config.state.normal = normal;
    return TFT_ST7735_OK;
}
tft_st7735_status_t tft_st7735_set_idle(tft_st7735_t* tft, bool idle){
    if(idle && !tft->config.state.normal) {
        return TFT_ST7735_INVALID_STATE; // IDMON has no effect while Partial is on
    }
    if(idle) {
        write_cmd(tft, IDLE_ON_MODE_CMD);
    } else {
        write_cmd(tft, IDLE_OFF_MODE_CMD);
    }
    tft->config.state.idle = idle;
    return TFT_ST7735_OK;
}

tft_st7735_status_t tft_st7735_set_invert(tft_st7735_t* tft, bool invert){
    if(invert) {
        write_cmd(tft, INVERT_ON_CMD);
    } else {
        write_cmd(tft, INVERT_OFF_CMD);
    }
    tft->config.state.inverted = invert;
    return TFT_ST7735_OK;
}

tft_st7735_status_t tft_st7735_set_colmod(tft_st7735_t *tft, tft_st7735_color_depth_t depth){
    if(!tft) return TFT_ST7735_INVALID_ARG;
    if(!tft->config.state.normal) return TFT_ST7735_INVALID_STATE; // COLMOD is unavailable in Partial Mode
    
    uint8_t ifpf;
    switch (depth) {
    case TFT_ST7735_COLOR_DEPTH_12BIT: ifpf = 0x03; break;
    case TFT_ST7735_COLOR_DEPTH_16BIT: ifpf = 0x05; break;
    case TFT_ST7735_COLOR_DEPTH_18BIT: ifpf = 0x06; break;
    default: return TFT_ST7735_INVALID_ARG;
    }

    write_cmd(tft, COLMOD_SET_CMD);
    write_data(tft, &ifpf, 1);

    return TFT_ST7735_OK;
}

static void pack_frmctr(uint8_t* out, tft_st7735_frame_rate_t r){
    out[0] = r.rtn & 0x0F;
    out[1] = r.fp  & 0x3F;
    out[2] = r.bp  & 0x3F;
}

tft_st7735_status_t tft_st7735_set_frmctr_normal(tft_st7735_t* tft, tft_st7735_frame_rate_t rate){
    uint8_t p[3];
    pack_frmctr(p, rate);
    write_cmd(tft, FRMCTR1_CMD);
    write_data(tft, p, 3);
    return TFT_ST7735_OK;
}

tft_st7735_status_t tft_st7735_set_frmctr_idle(tft_st7735_t* tft, tft_st7735_frame_rate_t rate){
    uint8_t p[3];
    pack_frmctr(p, rate);
    write_cmd(tft, FRMCTR2_CMD);
    write_data(tft, p, 3);
    return TFT_ST7735_OK;
}

tft_st7735_status_t tft_st7735_set_frmctr_partial(tft_st7735_t* tft, tft_st7735_frame_rate_t line_inversion, tft_st7735_frame_rate_t frame_inversion){
    uint8_t p[6];
    pack_frmctr(&p[0], line_inversion);
    pack_frmctr(&p[3], frame_inversion);
    write_cmd(tft, FRMCTR3_CMD);
    write_data(tft, p, 6);
    return TFT_ST7735_OK;
}

tft_st7735_status_t tft_st7735_set_pwctr1(tft_st7735_t* tft, tft_st7735_pwctr1_t pwctr1) {
    uint8_t p[2];
    p[0] = pwctr1.vrh & 0x1F;
    p[1] = 0x40 | ((pwctr1.ib_sel & 0x03) << 4);

    write_cmd(tft, PWCTR1_CMD);
    write_data(tft, p, 2);
    return TFT_ST7735_OK;
}

tft_st7735_status_t tft_st7735_set_pwctr2(tft_st7735_t* tft, tft_st7735_pwctr2_t pwctr2){
    write_cmd(tft, PWCTR2_CMD);
    write_data(tft, pwctr2.bt, 1);
    return TFT_ST7735_OK;
}

tft_st7735_status_t tft_st7735_set_pwctr3(tft_st7735_t* tft, tft_st7735_pwctr_mode_t mode) {
    uint8_t p[2];
    p[0] = mode.ap & 0x07;
    p[1] = mode.dc & 0x07;

    write_cmd(tft, PWCTR3_CMD);
    write_data(tft, p, 2);
    return TFT_ST7735_OK;
}

tft_st7735_status_t tft_st7735_set_pwctr4(tft_st7735_t* tft, tft_st7735_pwctr_mode_t mode) {
    uint8_t p[2];
    p[0] = mode.ap & 0x07;
    p[1] = mode.dc & 0x07;

    write_cmd(tft, PWCTR4_CMD);
    write_data(tft, p, 2);
    return TFT_ST7735_OK;
}

tft_st7735_status_t tft_st7735_set_pwctr5(tft_st7735_t* tft, tft_st7735_pwctr_mode_t mode) {
    uint8_t p[2];
    p[0] = mode.ap & 0x07;
    p[1] = mode.dc & 0x07;

    write_cmd(tft, PWCTR5_CMD);
    write_data(tft, p, 2);
    return TFT_ST7735_OK;
}

tft_st7735_status_t tft_st7735_set_pwctr6(tft_st7735_t* tft, tft_st7735_pwctr6_t mode) {
    uint8_t p[2];
    p[0] = ((mode.sapa & 0x07) << 4) | (mode.sapb & 0x07);
    p[1] = ((mode.sapc & 0x07) << 4) | (mode.dcd & 0x07);

    write_cmd(tft, PWCTR6_CMD);
    write_data(tft, p, 2);
    return TFT_ST7735_OK;
}

tft_st7735_status_t tft_st7735_set_vmctr1(tft_st7735_t* tft, tft_st7735_vmctr1_t vmctr1){
    uint8_t p[2]; 
    p[0] = vmctr1.vmh & 0x3F;
    p[1] = vmctr1.vml & 0x3F;

    write_cmd(tft, VMCTR1_CMD);
    write_data(tft, p, 2);
}

tft_st7735_status_t tft_st7735_set_gamma(tft_st7735_t* tft, const uint8_t gamma_pos[16], const uint8_t gamma_neg[16]) {
    uint8_t pos[16];
    for(int i = 0; i < 16; i++) {
        pos[i] = gamma_pos[i] & 0x3F;
    }
    write_cmd(tft, GMCTRP1_CMD);
    write_data(tft, pos, 16);

    uint8_t neg[16];
    for (int i = 0; i < 16; i++) {
        neg[i] = gamma_neg[i] & 0x3F;
    }
    write_cmd(tft, GMCTRN1_CMD);
    write_data(tft, neg, 16);

    return TFT_ST7735_OK;
}

tft_st7735_status_t tft_st7735_set_madctl(tft_st7735_t* tft, tft_st7735_orientation_t orientation) {
    uint8_t my, mx, mv;
    switch (orientation) {
        case TFT_ST7735_ORIENTATION_PORTRAIT:           my = 0; mx = 0; mv = 0; break;
        case TFT_ST7735_ORIENTATION_LANDSCAPE:          my = 0; mx = 1; mv = 1; break;
        case TFT_ST7735_ORIENTATION_PORTRAIT_INVERTED:  my = 1; mx = 1; mv = 0; break;
        case TFT_ST7735_ORIENTATION_LANDSCAPE_INVERTED: my = 1; mx = 0; mv = 1; break;
        default: return TFT_ST7735_INVALID_ARG;
    }

    uint8_t p = (my << 7) | (mx << 6) | (mv << 5); 
    write_cmd(tft, MEMORY_ACCESS_CTRL_CMD);
    write_data(tft, &p, 1);
    return TFT_ST7735_OK;
}

tft_st7735_status_t tft_st7735_initialize(tft_st7735_t* tft, const tft_st7735_config_t* cfg) {
    if (!tft || !cfg) return TFT_ST7735_INVALID_ARG;
    if (!cfg->io.spi_write || !cfg->io.set_dc || !cfg->io.set_reset || !cfg->io.delay_ms || !cfg->io.get_tick_ms)
        return TFT_ST7735_INVALID_ARG;
    
    tft->config = *cfg;
    tft->initialized = false;
    
    hw_reset(tft);

    set_addr_window(tft, 0, 0, 127, 159);

    tft_st7735_status_t status;

    if ((status = tft_st7735_set_frmctr_normal(tft, cfg->frame_rate.normal)) != TFT_ST7735_OK)  return status;
    if ((status = tft_st7735_set_frmctr_idle(tft, cfg->frame_rate.idle)) != TFT_ST7735_OK)      return status;
    if ((status = tft_st7735_set_frmctr_partial(tft, cfg->frame_rate.partial.line, cfg->frame_rate.partial.frame)) != TFT_ST7735_OK) return status;
    if ((status = tft_st7735_set_pwctr1(tft, cfg->power.pwctr1)) != TFT_ST7735_OK) return status;
    if ((status = tft_st7735_set_pwctr2(tft, cfg->power.pwctr2)) != TFT_ST7735_OK) return status;
    if ((status = tft_st7735_set_vmctr1(tft, cfg->vmctr1)) != TFT_ST7735_OK)       return status;
    if ((status = tft_st7735_set_gamma(tft, cfg->gamma_pos, cfg->gamma_neg)) != TFT_ST7735_OK) return status;
    if ((status = tft_st7735_set_sleep(tft, false)) != TFT_ST7735_OK) return status;
    if ((status = tft_st7735_set_colmod(tft, TFT_ST7735_COLOR_DEPTH_16BIT)) != TFT_ST7735_OK) return status;

    if ((status = tft_st7735_set_display(tft, true)) != TFT_ST7735_OK) return status;

    tft->initialized = true;
    return TFT_ST7735_OK;
}

/*Maybe there's a better approach?*/
tft_st7735_config_t tft_st7735_config_default(void) {
    return (tft_st7735_config_t){
        .frame_rate = {
            .normal  = { .rtn = 0x02, .fp = 0x2D, .bp = 0x2E },
            .idle    = { .rtn = 0x02, .fp = 0x2D, .bp = 0x2E },
            .partial = {
                .line  = { .rtn = 0x02, .fp = 0x2D, .bp = 0x2E },
                .frame = { .rtn = 0x02, .fp = 0x2D, .bp = 0x2E },
            },
        },
        .power = {
            .pwctr1 = { .vrh = 0x02, .ib_sel = 0x03 },
            .pwctr2 = { .bt = 0x05 },                    
            .pwctr_modes = {
                .normal       = { .ap = 0x01, .dc = 0x01 }, 
                .idle         = { .ap = 0x02, .dc = 0x07 }, 
                .partial      = { .ap = 0x02, .dc = 0x04 }, 
                .partial_idle = { .sapa = 0x01, .sapb = 0x01, .sapc = 0x01, .dcd = 0x05 },
            },
        },
        .vmctr1 = { .vmh = 0x51, .vml = 0x4D }, 
    };
}

tft_st7735_status_t tft_st7735_draw_rect(tft_st7735_t* tft, uint16_t x, uint16_t y,
                                          uint16_t w, uint16_t h, const uint8_t* pixels, size_t len) {
    if (x + w > tft->config.width || y + h > tft->config.height) return TFT_ST7735_INVALID_ARG;

    set_addr_window(tft, x, y, x + w - 1, y + h - 1);
    write_cmd(tft, RAMWR_CMD);
    write_data(tft, pixels, len);
    return TFT_ST7735_OK;
}