#include "bmp280_port_mock.h"
#include <string.h>

#define MOCK_MAX_QUEUED_READS 8
#define MOCK_MAX_READ_LEN     32

typedef struct {
    uint8_t data[MOCK_MAX_READ_LEN];
    uint32_t len;
} mock_read_entry_t;

static struct {
    mock_read_entry_t queue[MOCK_MAX_QUEUED_READS];
    uint32_t queue_head;
    uint32_t queue_count;

    uint32_t fail_count;

    uint8_t last_write_reg;
    uint8_t last_write_value;

    uint32_t tick_ms;
} mock_state;

static bmp280_status_t mock_i2c_write(uint8_t addr, const uint8_t *data, uint32_t len, void *ctx) {
    (void)addr; (void)ctx;

    if (mock_state.fail_count > 0) {
        mock_state.fail_count--;
        return BMP280_ERROR;
    }

    if (len >= 1) mock_state.last_write_reg = data[0];
    if (len >= 2) mock_state.last_write_value = data[1];

    return BMP280_OK;
}

static bmp280_status_t mock_i2c_write_read(uint8_t addr,
    const uint8_t *tx, uint32_t tx_len,
    uint8_t *rx, uint32_t rx_len,
    uint32_t timeout_ms, void *ctx)
{
    (void)addr; (void)tx; (void)tx_len; (void)timeout_ms; (void)ctx;

    if (mock_state.fail_count > 0) {
        mock_state.fail_count--;
        return BMP280_ERROR;
    }

    if (mock_state.queue_count == 0) {
        // No queued response — treat as a test setup error, fail loudly.
        return BMP280_ERROR;
    }

    mock_read_entry_t *entry = &mock_state.queue[mock_state.queue_head];
    uint32_t copy_len = (rx_len < entry->len) ? rx_len : entry->len;
    memcpy(rx, entry->data, copy_len);

    mock_state.queue_head = (mock_state.queue_head + 1) % MOCK_MAX_QUEUED_READS;
    mock_state.queue_count--;

    return BMP280_OK;
}

static void mock_delay_ms(uint32_t ms) {
    mock_state.tick_ms += ms;
}

static uint32_t mock_get_tick_ms(void) {
    return mock_state.tick_ms;
}

void bmp280_port_mock_init(bmp280_io_t *io) {
    io->i2c_write      = mock_i2c_write;
    io->i2c_write_read = mock_i2c_write_read;
    io->delay_ms        = mock_delay_ms;
    io->get_tick_ms      = mock_get_tick_ms;
    io->ctx             = NULL; // no real handle needed — all state is in mock_state
}

void bmp280_mock_queue_read(const uint8_t *data, uint32_t len) {
    if (mock_state.queue_count >= MOCK_MAX_QUEUED_READS) return; // queue full, drop silently
    if (len > MOCK_MAX_READ_LEN) len = MOCK_MAX_READ_LEN;

    uint32_t idx = (mock_state.queue_head + mock_state.queue_count) % MOCK_MAX_QUEUED_READS;
    memcpy(mock_state.queue[idx].data, data, len);
    mock_state.queue[idx].len = len;
    mock_state.queue_count++;
}

void bmp280_mock_fail_next(uint32_t count) {
    mock_state.fail_count = count;
}

uint8_t bmp280_mock_last_write_reg(void) {
    return mock_state.last_write_reg;
}

uint8_t bmp280_mock_last_write_value(void) {
    return mock_state.last_write_value;
}

void bmp280_mock_advance_ms(uint32_t ms) {
    mock_state.tick_ms += ms;
}

void bmp280_mock_reset(void) {
    memset(&mock_state, 0, sizeof(mock_state));
}