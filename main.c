//everything in main.c except the DMAFastCopy and UpdateSpriteMemory functions by Nathan Misner
//tonc_input.h by Jasper Vijn
//everything in defines.h copied from example code from "Programming the Game Boy Advance: The Unofficial Guide" by J. Harbour
//graphics by Nathan Misner

typedef unsigned short u16;
typedef unsigned int u32;
u16 key_curr, key_prev;
#include "tile.h"
#include "title.raw.h"
#include "tonc_input.h"
#include "rotations.h"
#include "defines.h"
#include <stdlib.h>


//create an array of 128 sprites equal to OAM
Sprite sprites[128];

int x1,y1,x2,y2,x3,y3,x4,y4; //coordinates of active piece

int main(void) {
    int isAtTitleScreen=1; //if game's at title screen
    int seed=0; //seed for randomizer
    int x, y;
    SetMode(3 | BG2_ENABLE);
    for(y = 0; y < 160; y++)
        for(x = 0; x < 240; x++)
            videoBuffer[y * 240 + x]=title_Bitmap[y * 240 + x];
    while (1) {
        key_poll();
        seed++;
        if (seed > 10000)
            seed=0;
        if (key_is_down(KEY_START)) {
            for(y = 0; y < 160; y++) {
                for(x = 0; x < 240; x++) {
                    videoBuffer[y * 240 + x]=0;
                }
            }
        break;
        }
    }
    srand(seed);
    int n;
    int charbase = 0;
    int screenbase = 31;
    bg2map = (unsigned short *)ScreenBaseBlock(screenbase);
        //set up background 0
	REG_BG2CNT = BG_COLOR256 | ROTBG_SIZE_256x256 |
        (charbase << CHAR_SHIFT) | (screenbase << SCREEN_SHIFT);
    //set the video mode--mode 2with sprites
	SetMode(2 | OBJ_ENABLE | OBJ_MAP_1D |BG2_ENABLE);
    //set the palette
	DMAFastCopy((void*)tilePalette, (void*)BGPaletteMem, 256, DMA_16NOW);

	//set the tile images
	DMAFastCopy((void*)tileData, (void*)CharBaseBlock(0), 256/2, DMA_32NOW);

	//copy the tile map into background 0
	DMAFastCopy((void*)tiles_Map, (void*)bg2map, 512, DMA_32NOW);
    //move all sprites offscreen to hide them
	for(n = 0; n < 128; n++) {
		sprites[n].attribute0 = 160;
		sprites[n].attribute1 = 240;
	}

    //set the sprite palette
	for(n = 0; n < 256; n++)
		SpritePal[n] = tilePalette[n];

	int i,j;
    //tetris board init
    int board[10][22]; //0=top/left, 21=bottom/right
    for (i=0; i < 10;i++) {
        for (j=0; j < 22;j++) {
            board[i][j]=0;
        }
    }
    //wall init
    for (i=2; i < 22; i++) {
        setBlock(10,i,2);
    }
    setBlock(16,9,3); //n
    setBlock(17,9,4); //e
    setBlock(18,9,5); //x
    setBlock(19,9,6); //t
    const int DIFFICULTY=12; //how often pieces fall
    int gravityCount=0; //counter for gravity
    //tgm keeps a 4 piece history for fairness
    int history[4]={5,5,4,4}; //1=I, 2=O, 3=T, 4=S, 5=Z, 6=J, 7=L, 8=locked
    int isPieceActive=0; //1 if a piece is active
    int nextPiece=0; //represents piece in next display
    int currentPiece=0; //piece currently in play
    int tempNext=0;  //next piece but without evenness correction
    int alreadyPressed=0; //turns to 1 if button has already been pressed that iteration
    int debounceCounter=0;
    int rotation=0; //rotation state: 0-3, 0-1, or 0 depending on the piece
    int lastRotation;
    int xOffset=4;
    int yOffset=21;
    const int MOVEMENT_DELAY=6; //how long it takes the pieces to move
    
    for (i=0; i < 7; i++) { //tries 6 times to come up with a number that's not in history
        tempNext=(rand() % 7)+1;
        int isGood=1;
        for (j=0; j < 4; j++) {
        if (history[j]==tempNext)
            isGood=0;
        }
        if (isGood==1) {    //if number's not in history, sets next piece and ends
            break;          //otherwise, gives up after 6 times
        }
    }
    currentPiece=tempNext;
    for (i=3; i > 0; i--) { //adds piece to block history
        history[i]=history[i-1];
    }
    history[0]=currentPiece;
    
	while(1) {
 	  key_poll(); //checks what keys are pressed
      //randomizer
      if (isPieceActive==0) {
        for (i=0; i < 7; i++) { //tries 6 times to come up with a number that's not in history
            tempNext=(rand() % 7)+1;
            int isGood=1;
            for (j=0; j < 4; j++) {
                if (history[j]==tempNext)
                    isGood=0;
            }
            if (isGood==1) {    //if number's not in history, sets next piece and ends
                break;          //otherwise, gives up after 6 times
            }
        }
        nextPiece=tempNext;
        for (i=3; i > 0; i--) { //adds piece to block history
            history[i]=history[i-1];
        }
        history[0]=nextPiece;
        isPieceActive=1;
        xOffset=4;
        yOffset=21;
        rotation=0;
        lastRotation=0;
      }
      //input
        if (key_is_down(KEY_DOWN)) {
            if ((y1<21 && y2<21 && y3 <21 && y4<21) && (board[x1][y1+1]==0 && board[x2][y2+1]==0 && board[x3][y3+1]==0 && board[x4][y4+1]==0)) {
                if (alreadyPressed==0) {
                    yOffset--;
                    alreadyPressed=1;
                }
            }
        }
        else if (key_is_down(KEY_LEFT)) {
            if ((x1 > 0 && x2 > 0 && x3 > 0 && x4 > 0) && (board[x1-1][y1]==0 && board[x2-1][y2]==0 && board[x3-1][y3]==0 && board[x4-1][y4]==0)) {
                if (alreadyPressed==0) {
                    xOffset--;
                    alreadyPressed=1;
                }
            }
        }
        
        else if (key_is_down(KEY_RIGHT)) {
            if ((x1 < 9 && x2 < 9 && x3 < 9 && x4 < 9) && (board[x1+1][y1]==0 && board[x2+1][y2]==0 && board[x3+1][y3]==0 && board[x4+1][y4]==0)) {
                if (alreadyPressed==0) {
                    xOffset++;
                    alreadyPressed=1;
                }
            }
        }
        else {
            debounceCounter=0;
        }
        if ((debounceCounter < MOVEMENT_DELAY) && (key_is_down(KEY_UP) || key_is_down(KEY_DOWN) || key_is_down(KEY_LEFT) || key_is_down(KEY_RIGHT))) {
            if (key_is_down(KEY_DOWN))
                debounceCounter+=2;
            else
                debounceCounter++;
        }
        else {
            debounceCounter=0;
            alreadyPressed=0;
        }
        if (key_is_down(KEY_A) && key_was_up(KEY_A)) {
            lastRotation=rotation;
            if (currentPiece==1 || currentPiece==3 || currentPiece==6 || currentPiece==7) {
                if (rotation >=0 && rotation < 4)
                    rotation--;
                if (rotation < 0)
                    rotation=3;
                if (rotation >= 4)
                    rotation=0;
            }
            if (currentPiece==4 || currentPiece==5) {
                if (rotation==0)
                    rotation=1;
                else
                    rotation=0;
            }
        }
        
        if (key_is_down(KEY_B) && key_was_up(KEY_B)) {
            lastRotation=rotation;
            if (currentPiece==1 || currentPiece==3 || currentPiece==6 || currentPiece==7) {
                if (rotation >=0 && rotation < 4)
                    rotation++;
                if (rotation < 0)
                    rotation=3;
                if (rotation >= 4)
                    rotation=0;
            }
            if (currentPiece==4 || currentPiece==5) {
                if (rotation==0)
                    rotation=1;
                else
                    rotation=0;
            }
        }
        //collision detection
        if (board[x1][y1]==1 || board[x2][y2]==1 || board[x3][y3]==1 || board[x4][y4]==1) {
                if ((board[x1+1][y1]==0 && board[x2+1][y2]==0 && board[x3+1][y3]==0 && board[x4+1][y4]==0) && (x1+1 < 10 && x2+1 < 10 && x3+1 < 10 && x4+1 < 10)) {
                    xOffset++;
                }
                else if ((board[x1-1][y1]==0 && board[x2-1][y2]==0 && board[x3-1][y3]==0 && board[x4-1][y4]==0) && (x1-1 >=0 && x2-1 >=0 && x3-1 >=0 && x4-1>=0)) {
                    xOffset--;
                }
                else {
                    rotation=lastRotation;
                }
        }
        
        //wallkicks
        if (x1 < 0 || x2 < 0 || x3 < 0 || x4 < 0) {
            x1++; x2++; x3++; x4++;
            xOffset++;
        }
        else if (x1 > 9 || x2 > 9 || x3 > 9 || x4 > 9) {
            x1--; x2--; x3--; x4--;
            xOffset--;
        }
        
        setPieceRotation(currentPiece, rotation, xOffset,yOffset,0,0);
        setPieceRotation(nextPiece,0,16,12,1,1); //next piece display (kinda hacky but whatever)
	    //draws active piece onscreen using sprites 
	    int spriteCount=0; //number of sprites being displayed
        tileSetup();
        sprites[spriteCount].attribute0 = COLOR_256 | (8*(y1-2));
        sprites[spriteCount].attribute1 = SIZE_16 | (8*x1);
        spriteCount++;
        sprites[spriteCount].attribute0 = COLOR_256 | (8*(y2-2));
        sprites[spriteCount].attribute1 = SIZE_16 | (8*x2);
        spriteCount++;
        sprites[spriteCount].attribute0 = COLOR_256 | (8*(y3-2));
        sprites[spriteCount].attribute1 = SIZE_16 | (8*x3);
        spriteCount++;
        sprites[spriteCount].attribute0 = COLOR_256 | (8*(y4-2));
        sprites[spriteCount].attribute1 = SIZE_16 | (8*x4);
        spriteCount=0;
       
	   //gravity
        if (gravityCount==DIFFICULTY) {
            if ((y1<21 && y2<21 && y3 <21 && y4<21) && (board[x1][y1+1]==0 && board[x2][y2+1]==0 && board[x3][y3+1]==0 && board[x4][y4+1]==0)) {
                if (key_is_up(KEY_DOWN))
                    yOffset--;
            }
            else if (board[x1][y1]==0 && board[x2][y2]==0 && board[x3][y3]==0 && board[x4][y4]==0) {
                isPieceActive=0;
                setPieceRotation(nextPiece,0,16,12,1,0);
                currentPiece=nextPiece;
                
                setBlock(x1,y1,1);
                setBlock(x2,y2,1);
                setBlock(x3,y3,1);
                setBlock(x4,y4,1);
                board[x1][y1]=1;
                board[x2][y2]=1;
                board[x3][y3]=1;
                board[x4][y4]=1;
                for (i=0; i < 10; i++) {
                    if (board[i][2]==1) {
                        for (i=0; i < 22; i++) {
                            for (j=0; j < 10; j++) {
                                board[j][i]=0;
                                setBlock(j,i,0);
                            }
                        }
                    }
                }
                for (i=0; i < 22; i++) {
                    int isRowFilled=1;
                    for (j=0; j < 10; j++) {
                        if (board[j][i]==0) {
                            isRowFilled=0;
                        }
                    }
                    if (isRowFilled==1) {
                        int k,l;
                        for (k=i; k > 0; k--) {
                            for (l=0; l < 10; l++) {
                                board[l][k]=board[l][k-1];
                                board[l][0]=0;
                                setBlock(l,k,board[l][k]);
                            }
                        }
                    }
                }
            }
            gravityCount=0;
        }
        else {
            gravityCount++;
        }
        //wait for vertical retrace
        WaitForVsync();

        //display the sprite
		UpdateSpriteMemory();
        
   	}
}


void WaitForVsync(void) {
    while (REG_VCOUNT >= 160);
    while (REG_VCOUNT < 160);
}

//copies sprite array into vram
void UpdateSpriteMemory(void) {
	int n;
	unsigned short* temp;
	temp = (unsigned short*)sprites;
	
	for(n = 0; n < 128*4; n++)
		SpriteMem[n] = temp[n];
}
void tileSetup(void) {
    int n;
    //copy the sprite image into memory
    for(n = 0; n < 32; n++) {
		SpriteData[n] = tileData[n];
	}
}
//sets block in bg tilemap (not actually x/y coordinates, but what the 2d array uses)
void setBlock(int x, int y, int val) {
    if (val==1)
        tiles_Map[(y-2)*32+x]=0;
    else if (val==0)
        tiles_Map[(y-2)*32+x]=1;
    else
        tiles_Map[(y-2)*32+x]=val;
    DMAFastCopy((void*)tiles_Map, (void*)bg2map, 512, DMA_32NOW);
}
//xOffset: how far block is away from the left
//yOffset: how far block is from the bottom
//rotation: 0-3, 0-1, or 0 depending on the piece
//isBg- 0 to use sprites, 1 to use background
//bgValue- 1 filled, 0 not filled
int setPieceRotation(int piece, int rotation, int xOffset, int yOffset, int isBg, int bgValue) { //1=I, 2=O, 3=T, 4=S, 5=Z, 6=J, 7=L
    int index;
    if (piece==1)
        index=rotation;
    else if (piece==2)
        index=4;
    else if (piece==3)
        index=5+rotation;
    else if (piece==4)
        index=9+rotation;
    else if (piece==5)
        index=11+rotation;
    else if (piece==6)
        index=13+rotation;
    else if (piece==7)
        index=17+rotation;
    int i,j;
    int* x1_ptr= &x1; //sorry, i'm garbage
    int* y1_ptr= &y1;
    int* x2_ptr= &x2;
    int* y2_ptr= &y2;
    int* x3_ptr= &x3;
    int* y3_ptr= &y3;
    int* x4_ptr= &x4;
    int* y4_ptr= &y4;
    int* pieceCoordinates[]={x1_ptr,y1_ptr,x2_ptr,y2_ptr,x3_ptr,y3_ptr,x4_ptr,y4_ptr};
    int count=0;
    //retrieves piece from array and places it on board
    for (i=0; i < 4; i++) {
        for (j=0; j < 4; j++) {
            if (rotations[index][j][i]==1) { //j,i due to my screwed up drawing code I don't want to fix
                if (isBg==0) {
                    *pieceCoordinates[count]=i+xOffset;
                    *pieceCoordinates[count+1]=21-yOffset+j;
                    if (count < 6) {
                        count +=2;
                    }
                }
                else {
                    setBlock(i+xOffset,21-yOffset+j,bgValue);
                }
            }
        }
    }
}
//fast copy function built into hardware
void DMAFastCopy(void* source, void* dest, unsigned int count, unsigned int mode) {
    if (mode == DMA_16NOW || mode == DMA_32NOW) {
    	REG_DMA3SAD = (unsigned int)source;
        REG_DMA3DAD = (unsigned int)dest;
        REG_DMA3CNT = count | mode;
    }
}

