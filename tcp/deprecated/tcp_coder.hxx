

namespace nginz
{

namespace tcp_coder
{
    int encode_header(tcp_segment_header&, uint8_t*buf, uint32_t size);
    int decode_header(tcp_segment_header&, uint8_t*buf, uint32_t size);
}

}

