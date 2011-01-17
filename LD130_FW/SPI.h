////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2010
//
////////////////////////////////////////////////////////////////////

#ifndef SPI_201156
#define SPI_201156


void initSPI1(void);
unsigned short SPI1_WriteRead(unsigned short data);


typedef enum tagSpiChanels {
	SPI_Chip_Select_None	= 0x00,
	SPI_Chip_Select_1		= 0x01,
	SPI_Chip_Select_2		= 0x02,
	SPI_Chip_Select_3		= 0x04,
	SPI_Chip_Select_4		= 0x08,
	SPI_Chip_Select_5		= 0x10,
} TSpiChanels;
//void SPI1_ChipSelect(unsigned char aChip);
void SPI1_ChipSelect_Single(unsigned char aChip, unsigned char bSelected);


void initSPI2(unsigned char b16Bit, unsigned char CKE, unsigned char CKP);

unsigned short SPI2_WriteRead(unsigned short data);
//void SPI2_ChipSelect(unsigned char aChip);
void SPI2_ChipSelect_Single(unsigned char aChip, unsigned char bSelected);

void SPI1_Set16BitMode(unsigned char is16BitMode);
#endif
