#pragma once

void setup(){
	tResizeWindow(30, 120);
	tDisable(T_QUICKEDIT);
	tViewport(0, 0, 30, 120);
	tHideCursor();
	tSetOutputCol(0x0F);
	tEnable(T_BLEND);
}
