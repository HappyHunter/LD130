////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2010
//
////////////////////////////////////////////////////////////////////

#ifndef SPI_201156
#define SPI_201156
typedef enum tagSpiChanels {
	SPI_Chip_Select_None	= 0x00,
	SPI_Chip_Select_1		= 0x01,
	SPI_Chip_Select_2		= 0x02,
	SPI_Chip_Select_3		= 0x04,
	SPI_Chip_Select_4		= 0x08,
	SPI_Chip_Select_5		= 0x10,
} TSpiChanels;


void initSPI1(unsigned char b16Bit, unsigned char CKE, unsigned char CKP);
unsigned short SPI1_WriteRead(unsigned short aData);
unsigned short SPI1_SelectWriteRead(unsigned char aChip, unsigned short aData);
unsigned short SPI1_NegSelectWriteRead(unsigned char aChip, unsigned short aData);



//void SPI1_ChipSelect(unsigned char aChip);
void SPI1_ChipSelect_Single(unsigned char aChip, unsigned char bSelected);



void initSPI2(unsigned char b16Bit, unsigned char CKE, unsigned char CKP);

unsigned short SPI2_WriteRead(unsigned short aData);
unsigned short SPI2_SelectWriteRead(unsigned char aChip, unsigned short aData);
unsigned short SPI2_NegSelectWriteRead(unsigned char aChip, unsigned short aData);

//void SPI2_ChipSelect(unsigned char aChip);
void SPI2_ChipSelect_Single(unsigned char aChip, unsigned char bSelected);

void SPI1_Set16BitMode(unsigned char is16BitMode);
#endif
