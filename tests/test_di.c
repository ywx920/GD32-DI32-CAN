#include "test_platform.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static uint8_t fram[512];
static uint32_t now_ms, input_state;
static unsigned int free_slots = 15U, writes, fail_write, fail_read;
static unsigned int blocked_channel = 2U;
static unsigned int sent_count;
static struct { unsigned int ch; uint32_t id; uint8_t data[8]; } sent[4096];
uint8_t FM_SendStr(uint16_t a, uint8_t *d, uint8_t n)
{
    ++writes;
    if (writes == fail_write) { return 0U; }
    assert(a + n <= sizeof(fram)); memcpy(fram + a, d, n); return 1U;
}
uint8_t FM_ReadStr(uint16_t a, uint8_t *d, uint8_t n)
{
    if (fail_read) { return 0U; }
    assert(a + n <= sizeof(fram)); memcpy(d, fram + a, n); return 1U;
}
uint32_t systick_millis(void) { return now_ms; }
uint32_t DIM_InputGetState(void) { return input_state; }
unsigned int can_tx_free(unsigned int ch) { return ch == blocked_channel ? 0U : free_slots; }
unsigned int can_channel_error_get(unsigned char ch) { (void)ch; return 0U; }
unsigned int send_can(unsigned char ch, unsigned int id, const unsigned char *data, unsigned char len)
{
    assert(len == 8U); assert(sent_count < 4096U);
    if (!can_tx_free(ch)) { return 0U; }
    sent[sent_count].ch = ch; sent[sent_count].id = id;
    memcpy(sent[sent_count].data, data, 8U); ++sent_count; return 1U;
}
static can_receive_message_struct request(uint8_t parameter, uint16_t value)
{
    can_receive_message_struct m;
    di_id id = {7U, 6U, DI_DEFAULT_ADDRESS, DI_DEFAULT_CHANNEL, 0U, 0U};
    memset(&m, 0, sizeof(m)); m.rx_ff = CAN_FF_EXTENDED;
    m.rx_dlen = 8U; m.rx_efid = di_id_encode(&id);
    m.rx_data[0] = parameter;
    m.rx_data[1] = (uint8_t)(value >> 8); m.rx_data[2] = (uint8_t)value;
    return m;
}
static void reset(void)
{
    memset(fram, 0xFF, sizeof(fram)); now_ms = 0U; input_state = 0U;
    writes = fail_write = fail_read = sent_count = 0U;
    free_slots = 15U; DI_TaskInit(now_ms);
    blocked_channel = 2U;
}
static void core_test(void)
{
    di_id in = {2U,11U,0U,27U,0U,0U}, out;
    di_config c, decoded, original;
    uint8_t data[20], corrupt[20]; uint32_t generation; unsigned int i, bit;
    assert(di_id_encode(&in) == 0x02581B00U);
    for (i = 0U; i < 32U; ++i) {
        in.type = (uint8_t)i; in.source = (uint8_t)(31U-i);
        in.destination = (uint8_t)i; in.channel = (uint8_t)(i*2U);
        in.slave = (uint8_t)(i%16U); in.sequence = (uint8_t)(15U-i%16U);
        di_id_decode(di_id_encode(&in), &out);
        assert(memcmp(&in, &out, sizeof(in)) == 0);
        di_status_pack(1UL << i, data);
        assert(data[3U-i/8U] == (uint8_t)(1U << (i%8U)));
        assert(data[4] == 0U && data[7] == 0U);
    }
    di_config_default(&c); original = c;
    assert(!di_parameter_update(&c,2U,0U)); assert(!di_parameter_update(&c,2U,64U));
    assert(!di_parameter_update(&c,3U,0U)); assert(!di_parameter_update(&c,3U,32U));
    assert(!di_parameter_update(&c,1U,500U)); assert(di_parameter_update(&c,1U,250U));
    assert(!di_parameter_update(&c,99U,1U)); assert(memcmp(&c,&original,sizeof(c))==0);
    assert(di_parameter_update(&c,4U,0U)); assert(di_parameter_update(&c,5U,65535U));
    di_config_encode(&c,0xFFFFFFFEU,data);
    assert(di_config_decode(data,&decoded,&generation));
    assert(generation==0xFFFFFFFEU && decoded.status_ms==65535U && decoded.heartbeat_ms==0U);
    for (i=0U;i<20U;++i) for(bit=0U;bit<8U;++bit) {
        memcpy(corrupt,data,20U); corrupt[i]^=(uint8_t)(1U<<bit);
        assert(!di_config_decode(corrupt,&decoded,&generation));
    }
}
static void periodic_test(void)
{
    unsigned int i;
    reset(); assert(!DI_ConfigLoaded && writes==0U);
    assert(DI_Config.address==DI_DEFAULT_ADDRESS && DI_Config.channel==DI_DEFAULT_CHANNEL);
    input_state=0x80000001U;
    DI_TaskPoll(1999U); assert(sent_count==0U);
    DI_TaskPoll(2000U); assert(sent_count==4U);
    assert(sent[0].id==0x04881C00U && sent[0].data[0]==0U);
    assert(sent[1].ch==1U && sent[1].data[0]==0U);
    assert(sent[2].id==0x02881C00U && sent[2].data[0]==0x80U && sent[2].data[3]==1U);
    DI_TaskPoll(4000U); assert(sent[4].data[0]==1U);
    for(i=3U;i<=257U;++i) DI_TaskPoll(i*2000U);
    assert(sent[1024].data[0]==0U); /* 8-bit counter rollover, same on both buses */
    reset(); DI_Config.heartbeat_ms=0U; DI_Config.status_ms=0U;
    DI_TaskPoll(99999U); assert(sent_count==0U);
    reset(); DI_TaskInit(0xFFFFFF00U); DI_TaskPoll(0x000006D0U);
    assert(sent_count==4U); /* unsigned tick rollover: 2000 ms elapsed */
    reset(); free_slots=4U; DI_TaskPoll(2000U); assert(sent_count==0U);
    free_slots=15U; DI_TaskPoll(2001U); assert(sent_count==0U);
    DI_TaskPoll(4000U); assert(sent_count==4U);
    reset(); blocked_channel=0U; DI_TaskPoll(2000U);
    assert(sent_count==2U && sent[0].ch==1U && sent[1].ch==1U);
}
static void parameter_test(void)
{
    can_receive_message_struct m; di_id reply;
    unsigned int before;
    reset(); m=request(5U,1234U); DI_ProcessCAN(1U,&m);
    assert(sent_count==1U && sent[0].ch==1U && sent[0].data[3]==1U);
    di_id_decode(sent[0].id,&reply);
    assert(reply.source==DI_DEFAULT_ADDRESS && reply.destination==6U &&
           reply.channel==DI_DEFAULT_CHANNEL && reply.sequence==0U);
    assert(DI_Config.status_ms==1234U && writes==3U && DI_ConfigLoaded);
    before=writes; DI_ProcessCAN(0U,&m);
    assert(writes==before && sent_count==2U); /* dual-bus repeated command */
    DI_TaskInit(0U); assert(DI_Config.status_ms==1234U && DI_ConfigLoaded);
    m=request(2U,29U); DI_ProcessCAN(0U,&m);
    assert(DI_Config.channel==29U && sent[sent_count-1U].data[3]==1U);
    now_ms=500U; before=writes; DI_ProcessCAN(1U,&m);
    assert(writes==before && sent[sent_count-1U].ch==1U); /* old channel retry */
    now_ms=4001U; before=sent_count; DI_ProcessCAN(0U,&m); assert(sent_count==before);
    reset(); m=request(3U,12U); DI_ProcessCAN(0U,&m);
    assert(DI_Config.address==12U); di_id_decode(sent[0].id,&reply);
    assert(reply.source==DI_DEFAULT_ADDRESS);
    DI_TaskInit(0U); assert(DI_Config.address==12U);
    reset(); m=request(1U,500U); DI_ProcessCAN(0U,&m);
    assert(sent_count==1U && sent[0].data[3]==0U && writes==0U);
    reset(); m=request(2U,64U); DI_ProcessCAN(0U,&m);
    assert(sent[0].data[3]==0U && DI_Config.channel==DI_DEFAULT_CHANNEL);
    reset(); m=request(4U,0U); DI_ProcessCAN(0U,&m); assert(DI_Config.heartbeat_ms==0U);
    DI_TaskPoll(2000U); assert(sent_count==3U); /* reply + status on two buses */
    reset(); m=request(5U,1U); m.rx_dlen=7U; DI_ProcessCAN(0U,&m);
    m.rx_dlen=8U; m.rx_ff=CAN_FF_STANDARD; DI_ProcessCAN(0U,&m);
    m.rx_ff=CAN_FF_EXTENDED; m.rx_ft=1U; DI_ProcessCAN(0U,&m);
    m.rx_ft=0U; m.rx_efid &= ~(31UL<<14); DI_ProcessCAN(0U,&m); /* broadcast */
    m=request(5U,1U); m.rx_efid ^= (1UL<<8); DI_ProcessCAN(0U,&m); /* other channel */
    m=request(5U,1U); m.rx_data[3]=1U; DI_ProcessCAN(0U,&m); /* response */
    assert(sent_count==0U && writes==0U);
    reset(); free_slots=0U; m=request(5U,1U); DI_ProcessCAN(0U,&m);
    assert(writes==0U && DI_Config.status_ms==2000U);
}
static void persistence_test(void)
{
    can_receive_message_struct m;
    unsigned int stage;
    for(stage=1U;stage<=3U;++stage) {
        reset(); m=request(5U,3000U); DI_ProcessCAN(0U,&m);
        assert(DI_Config.status_ms==3000U);
        now_ms=5000U; m=request(5U,4000U); fail_write=writes+stage;
        DI_ProcessCAN(0U,&m);
        assert(sent[sent_count-1U].data[3]==0U && DI_Config.status_ms==3000U);
        fail_write=0U; DI_TaskInit(0U); assert(DI_Config.status_ms==3000U);
    }
    reset(); m=request(5U,3000U); DI_ProcessCAN(0U,&m);
    now_ms=5000U; m=request(5U,4000U); fail_read=1U; DI_ProcessCAN(0U,&m);
    assert(sent[sent_count-1U].data[3]==0U && DI_Config.status_ms==3000U);
    fail_read=0U; DI_TaskInit(0U); assert(DI_Config.status_ms==3000U);
    reset(); m=request(5U,3000U); DI_ProcessCAN(0U,&m);
    now_ms=5000U; m=request(5U,4000U); DI_ProcessCAN(0U,&m);
    fram[0x120U+10U]^=1U; /* corrupt newer slot: fall back to previous */
    DI_TaskInit(0U); assert(DI_Config.status_ms==3000U);
}
int main(void)
{
    core_test(); periodic_test(); parameter_test(); persistence_test();
    puts("PASS: ID/32 DI bits/160 CRC corruptions/parameters/timing/rollover/routing/retries/storage failure recovery");
    return 0;
}
