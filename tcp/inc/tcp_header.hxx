
#ifndef NGINZ_TCP_HEADER_HXX
#define NGINZ_TCP_HEADER_HXX

namespace nginz
{
namespace tcp
{

namespace tcp_mask
{
constexpr uint16_t FIN = 1;
constexpr uint16_t SYN = 1<<1;
constexpr uint16_t RST = 1<<2;
constexpr uint16_t PSH = 1<<3;
constexpr uint16_t ACK = 1<<4;
constexpr uint16_t URG = 1<<5;
constexpr uint16_t ECE = 1<<6;
constexpr uint16_t CWR = 1<<7;
}

struct TcpSegmentHeader final
{
    //! \WARNING modifying this structure may cause tcp header corruption 
    uint16_t sport; //!< source port
    uint16_t dport; //!< destination port
    uint32_t seqno;
    uint32_t ackno;
    uint8_t  data_offset;
    uint8_t  evtmask;
    uint16_t supp_window_size; //!< it is recommended to be 32, while 16 bit is transmitted
    uint16_t checksum;
    uint16_t uptr; //!< urgent pointer
    uint32_t num_option_bytes = 0;
    uint32_t total_header_bytes = 0;

};


}
}


#endif // NGINZ_TCP_HEADER_HXX
