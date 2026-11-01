#pragma once

int focus = -1;
// 0 = url, 1 = main

namespace nsProcessNormalPage{
	bool isPressedLastFrame = false;
	bool isSelecting = false;
	pair<int, int> selectStart, selectEnd, cursorPos, mousePos, scroll;
	
	bool isMouseInArea = false;
	bool isPressingMouseLeft = false;
	
	File& file = curPage.curFile;
	
	pair<int, int> screenSpaceToTextSpace(pair<int, int> pr){
		return make_pair(pr.first - 2 + scroll.first, pr.second + scroll.second);
	}
	void setupMouseInfo(){
		mousePos = tGetMousePosChar();
		isPressingMouseLeft = GetAsyncKeyState(1) & 0x8000;
		isMouseInArea = tIsMouseInArea(2, 0, 26, 120);
	}
	void processMouseInput(){
		if(isMouseInArea){
			if(isPressingMouseLeft){
				selectEnd = screenSpaceToTextSpace(mousePos);
				if(!isPressedLastFrame){
					selectStart = screenSpaceToTextSpace(mousePos);
					focus = 1;
					cursorPos = screenSpaceToTextSpace(mousePos);
				}
			}
		}
		if(selectStart < selectEnd || selectEnd < selectStart){
			isSelecting = 1;
		}
		else{
			isSelecting = 0;
		}
		isPressedLastFrame = isPressingMouseLeft;
	}
	void clampPosOfAPair(pair<int, int>& pr, bool allowSelectAtEnd){
		if(file.lines.empty()) {
			file.lines = {""};
			pr = make_pair(0, 0);
			return;
		} 
		if(pr.first < 0) pr.first = 0;
		if(pr.first >= file.lines.size()) pr.first = file.lines.size() - 1;
		
		if(allowSelectAtEnd){
			if(pr.second > file.lines[pr.first].length()) pr.second = file.lines[pr.first].length();
		}
		else{
			if(pr.second >= file.lines[pr.first].length()) pr.second = file.lines[pr.first].length();
		}
		if(pr.second < 0) pr.second = 0;
	}
	void clampPos(){
		clampPosOfAPair(cursorPos, 1);
		clampPosOfAPair(selectStart, 0);
		clampPosOfAPair(selectEnd, 0);
	}
	// 									REMEMBER TO KEEP `scroll` POSITIVE
	void deleteSelect(){
		if(selectEnd < selectStart){
			swap(selectEnd, selectStart);
		}
		if(selectStart.first == selectEnd.first){
			file.lines[selectStart.first].erase(selectStart.second, selectEnd.second - selectStart.second + 1);
		}
		else{
			file.lines[selectStart.first].erase(selectStart.second);
			file.lines[selectStart.first] += file.lines[selectEnd.first].substr(selectEnd.second + 1);
			if(selectStart.first < selectEnd.first){
				file.erase(selectStart.first + 1, selectEnd.first - selectStart.first);
			}
		}
		isSelecting = 0;
		cursorPos = selectEnd = selectStart;
	}
	void processInput(){
		if(focus != 1 || !_kbhit()) return;
		
		wchar_t wch = _getwch();
		string chs = tWchar_to_gbk(wch);
		char ch = 65;
		if(chs.length() == 1) ch = chs[0];
		
		if(ch == '\b'){
			if(isSelecting){
				deleteSelect();
			}
			else{
				if(cursorPos.second > 0){
					file.lines[cursorPos.first].erase(--cursorPos.second, 1);
				}
				else if(cursorPos.first > 0){
					int len = file.lines[cursorPos.first - 1].length();
					file.lines[cursorPos.first - 1] += file.lines[cursorPos.first];
					file.erase(cursorPos.first, 1);
					cursorPos.first--; cursorPos.second = len;
				}
			}
		}
		else if(ch == '\r' || ch == '\n'){
			if(isSelecting) deleteSelect();
			file.insert(cursorPos.first + 1, file.lines[cursorPos.first].substr(cursorPos.second));
			file.lines[cursorPos.first].erase(cursorPos.second);
			cursorPos.first++; cursorPos.second = 0;
		}
		else if(wch == 224 || wch == 0){
			wch = _getwch();
			if(wch == 75){ // L Arrow
				if(isSelecting){
					if(selectEnd < selectStart) swap(selectStart, selectEnd);
					cursorPos = selectEnd = selectStart;
					isSelecting = 0;
				}
				if(cursorPos.second == 0){
					if(cursorPos.first > 0) cursorPos.second = file.lines[--cursorPos.first].length();
				}
				else cursorPos.second--;
			}
			else if(wch == 77){ // R Arrow
				if(isSelecting){
					if(selectEnd < selectStart) swap(selectStart, selectEnd);
					cursorPos = selectStart = selectEnd;
					isSelecting = 0;
				}
				if(cursorPos.second == file.lines[cursorPos.first].length()){
					if(cursorPos.first + 1 < file.lines.size()) cursorPos.second = 0, cursorPos.first++;
				}
				else cursorPos.second++;
			}
			else if(wch == 72){ // U Arrow
				// no-desiredCol is expected
				if(isSelecting){
					if(selectEnd < selectStart) swap(selectStart, selectEnd);
					cursorPos = selectEnd = selectStart;
					isSelecting = 0;
				}
				if(cursorPos.first > 0) --cursorPos.first,
				clampPos();
			}
			else if(wch == 80){ // D Arrow
				if(isSelecting){
					if(selectEnd < selectStart) swap(selectStart, selectEnd);
					cursorPos = selectStart = selectEnd;
					isSelecting = 0;
				}
				if(cursorPos.first + 1 < file.lines.size()) cursorPos.first++,
				clampPos();
			}
			else if(wch == 83){ // Delete
				if(isSelecting){
					deleteSelect();
				}
				else{
					if(cursorPos.second < file.lines[cursorPos.first].length()){
						file.lines[cursorPos.first].erase(cursorPos.second, 1);
					}
					else if(cursorPos.first + 1 < file.lines.size()){
						file.lines[cursorPos.first] += file.lines[cursorPos.first + 1];
						file.erase(cursorPos.first + 1, 1);
					}
				}
			}
			else if(wch == 71){ // Home
				if(isSelecting){
					if(selectEnd < selectStart) swap(selectStart, selectEnd);
					cursorPos = selectEnd = selectStart;
					isSelecting = 0;
				}
				cursorPos.second = 0;
			}
			else if(wch == 79){ // End
				if(isSelecting){
					if(selectEnd < selectStart) swap(selectStart, selectEnd);
					cursorPos = selectStart = selectEnd;
					isSelecting = 0;
				}
				cursorPos.second = file.lines[cursorPos.first].length();
			}
		}
		/*
		else{
			string inp = tWchar_to_gbk(wch);
			if(isSelecting){
				if(prEndMouse.second < prStartMouse.second){
					url.erase(prEndMouse.second, prStartMouse.second - prEndMouse.second + 1);
					url.insert(prEndMouse.second, inp);
					prCursorPos.second = prEndMouse.second + inp.length();
				} 
				else {
					url.erase(prStartMouse.second, prEndMouse.second - prStartMouse.second + 1);
					url.insert(prStartMouse.second, inp);
					prCursorPos.second = prStartMouse.second + inp.length();
				}
				isSelecting = 0;
				prStartMouse = prEndMouse = make_pair(0, 0);
			}
			else{
				url.insert(prCursorPos.second, inp);
				prCursorPos.second += inp.length();
			} 
		}
		*/
	}
	void processNormalPage(){
		setupMouseInfo();
		processMouseInput();
		clampPos();
		processInput();
	}
} 

void processManagerPage(){
	;
}
void processLoadingPage(){
	tSetOutputCol(0x09);
	tOutput("Loading... Please wait.", 10, 45);
	tOutput("加载中... 请等候。", 11, 45);
	tSetOutputCol(0x0F);
}
void process401Page(){
	tSetOutputCol(0x0C);
	if(curPage.desc == "invalid_token"){
		tOutput("401 Invalid Token", 10, 45);
		tOutput("401 token 无效", 11, 45);
		tOutput("You'll be forbidden after 3 times. (" + to_string(curPage.invalidTokenCount) + "/3)", 20, 45);
		tOutput("您将在 3 次后被封禁。(" + to_string(curPage.invalidTokenCount) + "/3)", 21, 45);
	}
	else if(curPage.desc == "did_not_login"){
		tOutput("401 You didn't login", 10, 45);
		tOutput("401 您未登录", 11, 45);
	}
	tSetOutputCol(0x0F);
}
void process403Page(){
	tSetOutputCol(0x0C);
	tOutput("403 Forbidden", 10, 45);
	tOutput("403 您被封禁", 11, 45);
	tSetOutputCol(0x0F);
}
void process404Page(){
	tSetOutputCol(0x0C);
	tOutput("404 Not Found", 10, 45);
	tOutput("404 未找到页面", 11, 45);
	tSetOutputCol(0x0F);
}

namespace nsProcessURLInput{
	bool bLastMouse = 0;
	bool isSelecting = 0;
	bool isUrlLoaded = 0;
	string url = "";
	pair<int, int> prStartMouse, prEndMouse, prCursorPos;
	
	void processURLInput(){
		// process mouse input
		pair<int, int> prCurMouse = tGetMousePosChar();
		bool bCurMouse = GetAsyncKeyState(1/*mouse left*/)&0x8000;
		bool imia = tIsMouseInArea(0, 0, 1, 120);
		int urlHeadLength = 11 + svIp.length();
		
		if(imia && bCurMouse) prEndMouse = prCurMouse;
		if(imia && !bLastMouse && bCurMouse) prStartMouse = prCurMouse, focus = 0, prCursorPos = prCurMouse;
		if(prStartMouse < prEndMouse || prEndMouse < prStartMouse) isSelecting = 1;
		else isSelecting = 0;
		bLastMouse = bCurMouse;
		
		// process url
		if(!isUrlLoaded){
			stringstream ssPath;
			ssPath << "a67note://" << svIp << "/" << curPage.path;
			url = ssPath.str();
			isUrlLoaded = 1;
		}
		
		
		if(prCursorPos.second > url.length()) prCursorPos.second = url.length();
		if(prCursorPos.second < urlHeadLength) prCursorPos.second = urlHeadLength;
		if(prStartMouse.second >= url.length()) prStartMouse.second = url.length() - 1;
		if(prStartMouse.second < urlHeadLength) prStartMouse.second = urlHeadLength;
		if(prEndMouse.second >= url.length()) prEndMouse.second = url.length() - 1;
		if(prEndMouse.second < urlHeadLength) prEndMouse.second = urlHeadLength;
		
		// input
		if(focus == 0 && _kbhit()){
			wchar_t wch = _getwch();
			string chs = tWchar_to_gbk(wch);
			char ch = 65;
			if(chs.length() == 1) ch = chs[0];
			
			if(ch == '\b'){
				if(isSelecting){
					if(prEndMouse.second < prStartMouse.second) url.erase(prEndMouse.second, prStartMouse.second - prEndMouse.second + 1);
					else url.erase(prStartMouse.second, prEndMouse.second - prStartMouse.second + 1);
					isSelecting = 0;
					prStartMouse = prEndMouse = make_pair(0, 0);
				}
				else{
					if(prCursorPos.second > urlHeadLength){
						url.erase(--prCursorPos.second, 1);
					}
				}
			}
			else if(ch == '\r'){
				if(url.length() <= urlHeadLength){
					curPage.type = 404;
				} 
				else{
					curPage.path = url.substr(urlHeadLength);
					curPage.getPageWithCurPath();
				}
			}
			else if(wch == 224){
				wch = _getwch();
				if(wch == 75){ // L Arrow
					prCursorPos.second--;
				}
				else if(wch == 77){ // R Arrow
					prCursorPos.second++;
				}
				else if(wch == 83){ // Delete
					if(isSelecting){
						if(prEndMouse.second < prStartMouse.second) url.erase(prEndMouse.second, prStartMouse.second - prEndMouse.second + 1);
						else url.erase(prStartMouse.second, prEndMouse.second - prStartMouse.second + 1);
						isSelecting = 0;
						prStartMouse = prEndMouse = make_pair(0, 0);
					}
					else{
						if(prCursorPos.second != url.length()) url.erase(prCursorPos.second, 1);
					}
				}
				else if(wch == 71){ // Home
					prCursorPos.second = urlHeadLength;
				}
				else if(wch == 79){ // End
					prCursorPos.second = url.length();
				}
			}
			else{
				string inp = tWchar_to_gbk(wch);
				if(isSelecting){
					if(prEndMouse.second < prStartMouse.second){
						url.erase(prEndMouse.second, prStartMouse.second - prEndMouse.second + 1);
						url.insert(prEndMouse.second, inp);
						prCursorPos.second = prEndMouse.second + inp.length();
					} 
					else {
						url.erase(prStartMouse.second, prEndMouse.second - prStartMouse.second + 1);
						url.insert(prStartMouse.second, inp);
						prCursorPos.second = prStartMouse.second + inp.length();
					}
					isSelecting = 0;
					prStartMouse = prEndMouse = make_pair(0, 0);
				}
				else{
					url.insert(prCursorPos.second, inp);
					prCursorPos.second += inp.length();
				} 
			}
		}
		
		if(prCursorPos.second > url.length()) prCursorPos.second = url.length();
		if(prCursorPos.second < urlHeadLength) prCursorPos.second = urlHeadLength;
		if(prStartMouse.second >= url.length()) prStartMouse.second = url.length() - 1;
		if(prStartMouse.second < urlHeadLength) prStartMouse.second = urlHeadLength;
		if(prEndMouse.second >= url.length()) prEndMouse.second = url.length() - 1;
		if(prEndMouse.second < urlHeadLength) prEndMouse.second = urlHeadLength;
		
		// output cursor
		if(focus == 0 && (time(0) & 1)){
//			wlog("DEBUG", "1");
			tSetOutputCol(0xAF);
			tDisable(T_BLEND);
			tOutput(" ", prCursorPos.first, prCursorPos.second);
			tEnable(T_BLEND);
		}
		if(focus == 0 && isSelecting){
			tSetOutputCol(0x1F);
			tDisable(T_BLEND);
			if(prEndMouse.second < prStartMouse.second) tOutput(string(prStartMouse.second - prEndMouse.second + 1, ' '), 0, prEndMouse.second);
			else tOutput(string(prEndMouse.second - prStartMouse.second + 1, ' '), 0, prStartMouse.second);
			tEnable(T_BLEND);
		}
		tSetOutputCol(0x0F);
		
		
		// output  
		
		tOutput(url, 0, 0);
		tOutput(string(120, '='), 1, 0);
	}
}


void processPage(){
	tClear({' ', 0x0F});
	nsProcessURLInput::processURLInput();
	
	if(curPage.type == 0) nsProcessNormalPage::processNormalPage();
	else if(curPage.type == 1) processManagerPage();
	else if(curPage.type == 2) processLoadingPage();
	else if(curPage.type == 401) process401Page();
	else if(curPage.type == 403) process403Page();
	else if(curPage.type == 404) process404Page();
	tSwapBuffer();
}
