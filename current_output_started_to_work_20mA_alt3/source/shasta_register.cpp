#include	"shasta_register.h"

SHASTA_basic::SHASTA_basic( SPI& spi, bool spi_addr ) : 
	SPI_for_AFE( spi, spi_addr )
{
}

SHASTA_basic::~SHASTA_basic()
{
}


void SHASTA_basic::command( uint16_t com )
{
	write_r16( com );
}

void SHASTA_basic::reg( Register16 r, uint16_t value )
{
	write_r16( static_cast<uint16_t>( r ), value );
}

void SHASTA_basic::reg( Register24 r, uint32_t value )
{
	write_r24( static_cast<uint16_t>( r ), value );
}

uint16_t SHASTA_basic::reg( Register16 r )
{
	return read_r16( static_cast<uint16_t>( r ) );
}

uint32_t SHASTA_basic::reg( Register24 r )
{
	return read_r24( static_cast<uint16_t>( r ) );
}

