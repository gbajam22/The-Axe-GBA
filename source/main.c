#include <tonc.h>
#include <string.h>
#include <stdlib.h>

#include "titleScreen.h"
#include "gameOverScreen.h"
#include "walker.h"
#include "axe.h"

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 160

OBJ_ATTR obj_buffer[128];
OBJ_AFFINE *obj_affine_buffer = (OBJ_AFFINE*)obj_buffer;

typedef struct screenObjects {
	OBJ_ATTR *o_object;
	int x;
	int y;
	int width;
	int height;
	int isWalking;
	int isFacingEast;
	int velocity;
	u32 tid;
	u32 pb;
} screenObjects;

screenObjects initPlayer(screenObjects mPL, int x_pos, int y_pos, int wd, int ht, int iW, int iF, int vel, u32 tile_id, u32 pal_bank);
screenObjects initAxe(screenObjects mAx, int x_pos, int y_pos, int wd, int ht, int iW, int iF, int vel, u32 tile_id, u32 pal_bank);

void titleInit();
void gameOverInit();
void loadSprites();
void gameScreen();
void gameOverScreen();

int main(){
	titleInit();
	REG_DISPCNT = DCNT_MODE3 | DCNT_BG2;

	while(1){
		vid_vsync();
		key_poll();

		if(key_hit(KEY_START)){
			gameScreen();
		}
	}

	return 0;
}

void titleInit(){
	memcpy(vid_mem, titleScreenBitmap, titleScreenBitmapLen);
}

void gameOverInit(){
	memcpy(vid_mem, gameOverScreenBitmap, gameOverScreenBitmapLen);
}

void loadSprites(){
	memcpy(&tile_mem[4][0], walkerTiles, walkerTilesLen);
	memcpy(&tile_mem[4][16], axeTiles, axeTilesLen);

	memcpy(pal_obj_mem, walkerPal, walkerPalLen);
	memcpy(pal_obj_mem + 16, axePal, axePalLen);

	oam_init(obj_buffer, 128);
}

screenObjects initPlayer(screenObjects mPL, int x_pos, int y_pos, int wd, int ht, int iW, int iF, int vel, u32 tile_id, u32 pal_bank){
	mPL.o_object = &obj_buffer[0];
	mPL.x = x_pos;
	mPL.y = y_pos;
	mPL.width = wd;
	mPL.height = ht;
	mPL.isWalking = iW;
	mPL.isFacingEast = iF;
	mPL.velocity = vel;
	mPL.tid = tile_id;
	mPL.pb = pal_bank;

	obj_set_attr(mPL.o_object, ATTR0_SQUARE, ATTR1_SIZE_16x16, ATTR2_PALBANK(mPL.pb)|mPL.tid);
	obj_set_pos(mPL.o_object, mPL.x, mPL.y);

	return mPL;
}

screenObjects initAxe(screenObjects mAx, int x_pos, int y_pos, int wd, int ht, int iW, int iF, int vel, u32 tile_id, u32 pal_bank){
	mAx.o_object = &obj_buffer[1];
	mAx.x = x_pos;
	mAx.y = y_pos;
	mAx.width = wd;
	mAx.height = ht;
	mAx.isWalking = iW;
	mAx.isFacingEast = iF;
	mAx.velocity = vel;
	mAx.tid = tile_id;
	mAx.pb = pal_bank;

	obj_set_attr(mAx.o_object, ATTR0_SQUARE, ATTR1_SIZE_32x32, ATTR2_PALBANK(mAx.pb)|mAx.tid);
	obj_set_pos(mAx.o_object, mAx.x, mAx.y);

	return mAx;
}

void gameScreen(){
	screenObjects player;
	screenObjects axe;
	
	loadSprites();
	REG_DISPCNT = DCNT_OBJ | DCNT_OBJ_1D | DCNT_MODE0;

	player = initPlayer(player, 0, 72, 16, 16, 0, 1, 2, 0, 0);
	axe = initAxe(axe, 105, 0, 0, 32, 32, 0, 5, 16, 1);

	int frame = 0;

	int leftPlayer = 0;
	int rightPlayer = 0;
	int basePlayer = 0;
	int topPlayer = 0;

	int leftAxe = 0;
	int rightAxe = 0;
	int baseAxe = 0;
	int topAxe = 0;

	while(1){
		vid_vsync();
		key_poll();

		player.x += player.velocity * key_tri_horz();

		leftPlayer = player.x;
		rightPlayer = player.x + player.width;
		topPlayer = player.y;
		basePlayer = player.y + player.height;

		leftAxe = axe.x;
		rightAxe = axe.x + axe.width;
		topAxe = axe.y;
		baseAxe = axe.y + axe.height;

		if(key_tri_horz()){
			player.isWalking = 1;
			if(player.isWalking == 1){
				if((frame & 7) == 0){
					player.tid = (player.tid + 4) % 16;
					if(player.tid == 0){
						player.tid = 4;
					}
				}
				else if(key_curr_state() == KEY_LEFT){
					player.isFacingEast = 0;
				}
				else if(key_curr_state() == KEY_RIGHT){
					player.isFacingEast = 1;
				}
				frame++;
			}
		}
		else{
			player.isWalking = 0;
			player.tid = 0;
		}

		player.o_object->attr1 = (player.isFacingEast ? 0x4000 : 0x5000);

		axe.y = axe.y + axe.velocity;
		if(axe.y >= (SCREEN_HEIGHT - axe.height) || axe.y <= 0){
			axe.velocity = -axe.velocity;
		}

		if(player.x >= SCREEN_WIDTH - player.width){
			player.x = SCREEN_WIDTH - player.width;
		}
		if(player.x <= 0){
			player.x = 0;
		}

		if((baseAxe > topPlayer) && (topAxe < basePlayer) && (leftAxe < rightPlayer) && (rightAxe > leftPlayer)){
			gameOverScreen();
		}

		player.o_object->attr2 = ATTR2_BUILD(player.tid, player.pb, 0);
		axe.o_object->attr2 = ATTR2_BUILD(axe.tid, axe.pb, 0);

		obj_set_pos(player.o_object, player.x, player.y);
		obj_set_pos(axe.o_object, axe.x, axe.y);
		oam_copy(oam_mem, obj_buffer, 2);
	}
}

void gameOverScreen(){
	gameOverInit();
	REG_DISPCNT = DCNT_MODE3 | DCNT_BG2;

	while(1){
		vid_vsync();
		key_poll();

		if(key_hit(KEY_START)){
			gameScreen();
		}
		if(key_hit(KEY_SELECT)){
			main();
		}
	}
}