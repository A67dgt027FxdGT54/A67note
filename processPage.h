#pragma once

int focus = -1;
// 0 = url, 1 = main


/*    
 *  TIPS:
 *		cursorPos is the pos of char, instead of the gap.
 *      e.g. when cursorPos = (0, 1):
 *			0 1 2 3
 *          a[b]c d
 *           ~~~ cursor block
 *			 insert:
 *			0 1 2 3 4
 *			a i[b]c d
 *             ~~~ cursor block
 */

namespace nsProcessNormalPage{
	bool isPressedLastFrame = false;
	bool isSelecting = false;
	pair<int, int> selectStart, selectEnd, cursorPos, mousePos, scroll;
	
	bool isMouseInArea = false;
	bool isPressingMouseLeft = false;
	
	File& file = curPage.curFile;
	
	vector<tButton_t> btns = vector<tButton_t>{
		tButton_t(29, 0, "剪切", 0x0F),
		tButton_t(29, 6, "复制", 0x0F),
		tButton_t(29, 12, "粘贴", 0x0F),
		tButton_t(29, 18, "全选", 0x0F),
		tButton_t(29, 24, "保存", 0x0F),
		tButton_t(29, 30, "刷新", 0x0F),
		tButton_t(29, 36, "向上滚动", 0x0F),
		tButton_t(29, 46, "向下滚动", 0x0F),
		tButton_t(29, 56, "向左滚动", 0x0F),
		tButton_t(29, 66, "向右滚动", 0x0F)
	};
	
	pair<int, int> screenSpaceToTextSpace(pair<int, int> pr){
		return make_pair(pr.first - 2 + scroll.first, pr.second + scroll.second);
	}
	
	pair<int, int> textSpaceToScreenSpace(pair<int, int> pr){
		return make_pair(pr.first + 2 - scroll.first, pr.second - scroll.second);
	}
	
	string alertMsg = "";
	tCol alertCol = 0x0F; 
	void processAlert(){
		int oCol = tOutputCol;
		tSetOutputCol(alertCol);
		tOutput(
			alertMsg,
			29,
			120 - alertMsg.length()
		);
		tSetOutputCol(oCol);
	}
	void alert(tCol col, const string& msg){
		alertCol = col;
		alertMsg = msg;
	}
	
	void setupMouseInfo(){
//		wlog("DEBUG", "call setupMouseInfo()");
		mousePos = tGetMousePosChar();
		isPressingMouseLeft = GetAsyncKeyState(1) & 0x8000;
		isMouseInArea = tIsMouseInArea(2, 0, 26, 120);
	}
	void processMouseInput(){
//		wlog("DEBUG", "call processMouseInput()");
//		wlog("DEBUG", "processMouseInput() mouse key");
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
//		wlog("DEBUG", "processMouseInput() select");
		if(selectStart < selectEnd || selectEnd < selectStart){
			isSelecting = 1;
		}
		else{
			isSelecting = 0;
		}
		isPressedLastFrame = isPressingMouseLeft;
//		wlog("DEBUG", "processMouseInput() end");
	}
	void clampPosOfAPair(pair<int, int>& pr, bool allowSelectAtEnd){
		if(file.lines.empty()) {
			file.lines = {""};
			file.charType = {{}};
			pr = make_pair(0, 0);
			return;
		} 
		if(pr.first < 0) pr.first = 0;
		if(pr.first >= file.lines.size()) pr.first = file.lines.size() - 1;
		
		if(allowSelectAtEnd){
			if(pr.second > file.lines[pr.first].length()) pr.second = file.lines[pr.first].length();
		}
		else{
			if(pr.second >= file.lines[pr.first].length()) pr.second = static_cast<int>(file.lines[pr.first].length()) - 1;
		}
		if(pr.second < 0) pr.second = 0;
	}
	void clampPos(){
//		wlog("DEBUG", "call clampPos()");
//		wlog("DEBUG", "clampPos() clamp pos of a pair");
		clampPosOfAPair(cursorPos, 1);
		clampPosOfAPair(selectStart, 0);
		clampPosOfAPair(selectEnd, 0);
//		wlog("DEBUG", "clampPos() clamp select");
		if(selectStart < selectEnd){
			if(
				selectStart.second > 0 && 
				selectStart.second < file.charType[selectStart.first].size() && 
				file.charType[selectStart.first][selectStart.second] == 2
			) selectStart.second--;
			
			if(
				selectEnd.second > 0 && 
				selectEnd.second < file.charType[selectEnd.first].size() &&
				file.charType[selectEnd.first][selectEnd.second] == 1
			) selectEnd.second--;
		} 
		else if(selectStart == selectEnd){
//			if(
//				selectEnd.second > 0 &&
//				selectEnd.second < file.lines[selectEnd.first].length() &&
//				file.charType[selectEnd.first][selectEnd.second] == 1
//			){
//				selectEnd.second--;
//			}
//			else if(
//				selectStart.second > 0 && 
//				selectStart.second + 1 < file.charType[selectStart.first].size() && 
//				file.charType[selectStart.first][selectStart.second] == 2
//			){
//				selectStart.second++;
//			}
			selectStart = selectEnd = cursorPos;
			isSelecting = 0;
		}
		else {
			if(
				selectStart.second > 0 && 
				selectStart.second < file.charType[selectStart.first].size() && 
				file.charType[selectStart.first][selectStart.second] == 1
			) selectStart.second--;
			
			if(
				selectEnd.second > 0 && 
				selectEnd.second < file.charType[selectEnd.first].size() &&
				file.charType[selectEnd.first][selectEnd.second] == 2
			) selectEnd.second--;
		} 
//		wlog("DEBUG", "clampPos() clamp cursorPos");
		if(cursorPos.second < file.charType[cursorPos.first].size() && file.charType[cursorPos.first][cursorPos.second] == 2){
			cursorPos.second++;
		}
//		wlog("DEBUG", "clampPos() clamp end");
	}
	// 									REMEMBER TO KEEP `scroll` POSITIVE
	void deleteSelect(){
//		wlog("DEBUG", "call deleteSelect()");
		if(selectEnd < selectStart){
			swap(selectEnd, selectStart);
		}
		if(selectStart.first == selectEnd.first){
			file.lines[selectStart.first].erase(selectStart.second, selectEnd.second - selectStart.second + 1);
			file.charType[selectStart.first].erase(
				file.charType[selectStart.first].begin() + selectStart.second, 
				file.charType[selectStart.first].begin() + selectEnd.second + 1
			);
		}
		else{
			file.lines[selectStart.first].erase(selectStart.second);
			file.charType[selectStart.first].erase(
				file.charType[selectStart.first].begin() + selectStart.second, 
				file.charType[selectStart.first].end()
			);
			if(selectEnd.second + 1 < file.lines[selectEnd.first].length())
				file.lines[selectStart.first] += file.lines[selectEnd.first].substr(selectEnd.second + 1), 
				file.charType[selectStart.first].insert(
					file.charType[selectStart.first].end(),
					file.charType[selectEnd.first].begin() + selectEnd.second + 1,
					file.charType[selectEnd.first].end()
				);
			if(selectStart.first < selectEnd.first){
				file.erase(selectStart.first + 1, selectEnd.first - selectStart.first);
			}
		}
		isSelecting = 0;
		cursorPos = selectEnd = selectStart;
	}
	void processInput(){
//		wlog("DEBUG", "call processInput()");
		if(focus != 1 || !tKbhit()) return;
		
		wchar_t wch = tGetwch();
		
		string chs = tWchar_to_gbk(wch);
		char ch = 65;
		if(chs.length() == 1) ch = chs[0];
		
		if(ch == '\b'){
			if(isSelecting){
				deleteSelect();
			}
			else{
				if(cursorPos.second > 0){
					if(file.charType[cursorPos.first][cursorPos.second - 1] == 0)
						file.lines[cursorPos.first].erase(--cursorPos.second, 1),
						file.charType[cursorPos.first].erase(
							file.charType[cursorPos.first].begin() + cursorPos.second,
							file.charType[cursorPos.first].begin() + cursorPos.second + 1
						);
					else if(file.charType[cursorPos.first][cursorPos.second - 1] == 2){
						cursorPos.second -= 2;
						file.lines[cursorPos.first].erase(cursorPos.second, 2);
						file.charType[cursorPos.first].erase(
							file.charType[cursorPos.first].begin() + cursorPos.second,
							file.charType[cursorPos.first].begin() + cursorPos.second + 2
						);
					}
				}
				else if(cursorPos.first > 0){
					int len = file.lines[cursorPos.first - 1].length();
					file.lines[cursorPos.first - 1] += file.lines[cursorPos.first];
					file.charType[cursorPos.first - 1].insert(
						file.charType[cursorPos.first - 1].end(),
						file.charType[cursorPos.first].begin(),
						file.charType[cursorPos.first].end()
					);
					file.erase(cursorPos.first, 1);
					cursorPos.first--; cursorPos.second = len;
				}
			}
		}
		else if(ch == '\r' || ch == '\n'){
			if(isSelecting) deleteSelect();
			if(cursorPos.second == file.lines[cursorPos.first].size()){
				file.insert(cursorPos.first + 1, "");
			}
			else{
				file.insert(cursorPos.first + 1, file.lines[cursorPos.first].substr(cursorPos.second));
				file.lines[cursorPos.first].erase(cursorPos.second);
				file.charType[cursorPos.first].erase(
					file.charType[cursorPos.first].begin() + cursorPos.second, 
					file.charType[cursorPos.first].end()
				);
			}
			cursorPos.first++; cursorPos.second = 0;
		}
		else if(wch == 224 || wch == 0){
			wch = tGetwch();
			if(wch == 75){ // L Arrow
				if(isSelecting){
					if(selectEnd < selectStart) swap(selectStart, selectEnd);
					cursorPos = selectEnd = selectStart;
					isSelecting = 0;
				}
				if(cursorPos.second == 0){
					if(cursorPos.first > 0) cursorPos.second = file.lines[--cursorPos.first].length();
				}
				else{
					if(cursorPos.second > 0 && file.charType[cursorPos.first][cursorPos.second - 1] == 0) cursorPos.second--;
					else cursorPos.second -= 2;
				}
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
				else{
					if(
						cursorPos.second + 1 == file.lines[cursorPos.first].length() || 
						file.charType[cursorPos.first][cursorPos.second] == 0
					) cursorPos.second++;
					else cursorPos.second += 2;
				}
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
						if(cursorPos.second + 1 < file.charType[cursorPos.first].size() && file.charType[cursorPos.first][cursorPos.second] == 1){
							file.lines[cursorPos.first].erase(cursorPos.second, 2);
							file.charType[cursorPos.first].erase(
								file.charType[cursorPos.first].begin() + cursorPos.second,
								file.charType[cursorPos.first].begin() + cursorPos.second + 2
							);
						} 
						else{
							file.lines[cursorPos.first].erase(cursorPos.second, 1);
							file.charType[cursorPos.first].erase(
								file.charType[cursorPos.first].begin() + cursorPos.second,
								file.charType[cursorPos.first].begin() + cursorPos.second + 1
							);
						}
					}
					else if(cursorPos.first + 1 < file.lines.size()){
						file.lines[cursorPos.first] += file.lines[cursorPos.first + 1];
						file.charType[cursorPos.first].insert(
							file.charType[cursorPos.first].end(),
							file.charType[cursorPos.first + 1].begin(),
							file.charType[cursorPos.first + 1].end()
						);
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
		else{
			string inp = tWchar_to_gbk(wch);
			if(inp == "\t") inp = "    ";
			auto ct = calcCharType(inp);
			if(isSelecting){
				deleteSelect(); 
			}
			file.lines[cursorPos.first].insert(cursorPos.second, inp);
			file.charType[cursorPos.first].insert(
				file.charType[cursorPos.first].begin() + cursorPos.second,
				ct.begin(),
				ct.end()
			);
			cursorPos.second += inp.length();
		}
	}
	void outputCursor(){
//		wlog("DEBUG", "call outputCursor()");
		pair<int, int> cp = textSpaceToScreenSpace(cursorPos);
		if(focus == 1 && (time(0) & 1)){
			tDisable(T_BLEND);
			tSetOutputCol(0xAF);
			if(cursorPos.second < file.charType[cursorPos.first].size() && file.charType[cursorPos.first][cursorPos.second] == 1){
				tOutput("  ", cp.first, cp.second);
			}
			else tOutput(" ", cp.first, cp.second);
			tEnable(T_BLEND);
		}
	}
	void outputSelect(){
//		wlog("DEBUG", "call outputSelect()");
		if(focus == 1 && isSelecting){
			tDisable(T_BLEND);
			tSetOutputCol(0x1F);
			pair<int, int> s = textSpaceToScreenSpace(selectStart), e = textSpaceToScreenSpace(selectEnd);
			bool bSwap = 0;
			if(s > e) swap(s, e), bSwap = 1;
			if(s.first == e.first){
				tOutput(
					string(e.second - s.second + 1, ' '),
					s.first, 
					s.second
				);
			}
			else{
				int se = textSpaceToScreenSpace(
					make_pair(0, bSwap? file.lines[selectEnd.first].length(): file.lines[selectStart.first].length())
				).second,
				es = textSpaceToScreenSpace(make_pair(0, 0)).second;
//				try{
				tOutput(string(max(0, se - s.second + 1), ' '), s.first, s.second);
				tOutput(string(max(0, e.second - es + 1), ' '), e.first, es);
//				}catch(std::length_error){
//					cout << "@";
//					throw;
//				}
				
				for(int i = s.first + 1; i < e.first; i++){
					int l = screenSpaceToTextSpace(make_pair(i, 0)).first;
					tOutput(
						string(file.lines[l].length() + 1, ' '),
						i,
						0
					);
				}
			}
			tEnable(T_BLEND);
		}
	}
	void outputText(){
//		wlog("DEBUG", "call outputText()");
		for(int i = 0; i < file.lines.size(); i++){
			pair<int, int> pos = textSpaceToScreenSpace(make_pair(i, 0));
			tOutput(file.lines[i], pos.first, pos.second);
			if(scroll.second < file.charType[i].size() && file.charType[i][scroll.second] == 2){
				tSetOutputCol(0x0B);
				tOutput("-", pos.first, 0);
				tSetOutputCol(0x0F);
			}
			if(scroll.second + 119 < file.charType[i].size() && file.charType[i][scroll.second + 119] == 1){
				tSetOutputCol(0x0B);
				tOutput("-", pos.first, 119);
				tSetOutputCol(0x0F);
			}
		}
	}
	void cutCallback(){
		if(!isSelecting || selectStart == selectEnd) return;
		
		string cbt;
		if(selectStart > selectEnd) swap(selectStart, selectEnd);
		
		if(selectStart.first == selectEnd.first) cbt = file.lines[selectStart.first].substr(
			selectStart.second, 
			selectEnd.second - selectStart.second + 1
		);
		else{
			cbt += file.lines[selectStart.first].substr(selectStart.second) + "\n";
			for(int i = selectStart.first + 1; i < selectEnd.first; i++){
				cbt += file.lines[i] + "\n";
			}
			cbt += file.lines[selectEnd.first].substr(0, selectEnd.second + 1);
		}
		tSetClipboardTextGbk(cbt);
		deleteSelect();
		alert(0x0A, "剪切成功");
	}
	void copyCallback(){
		if(!isSelecting || selectStart == selectEnd) return;
		
		string cbt;
		if(selectStart > selectEnd) swap(selectStart, selectEnd);
		
		if(selectStart.first == selectEnd.first) cbt = file.lines[selectStart.first].substr(
			selectStart.second, 
			selectEnd.second - selectStart.second + 1
		);
		else{
			cbt += file.lines[selectStart.first].substr(selectStart.second) + "\n";
			for(int i = selectStart.first + 1; i < selectEnd.first; i++){
				cbt += file.lines[i] + "\n";
			}
			cbt += file.lines[selectEnd.first].substr(0, selectEnd.second + 1);
		}
		tSetClipboardTextGbk(cbt);
		alert(0x0A, "复制成功");
	}
	void pasteCallback(){
		if(isSelecting) deleteSelect();
		string cb = tGetClipboardTextGbk();
		cb = replaceTabAndRemoveCR(cb);
		vector<string> lns;
		stringstream ss(cb);
		string ln;
		while(getline(ss, ln)) lns.push_back(ln);
		if(lns.size() == 1){
			vector<int> ct = calcCharType(lns[0]);
			file.lines[cursorPos.first].insert(cursorPos.second, lns[0]);
			file.charType[cursorPos.first].insert(
				file.charType[cursorPos.first].begin() + cursorPos.second,
				ct.begin(),
				ct.end()
			);
			cursorPos.second += lns[0].length();
		}
		else if(lns.size() >= 2){
			/**********************************************\
			                  FIRST LINE
			\**********************************************/
			
			if(cursorPos.second < file.lines[cursorPos.first].size()){
				file.insert(cursorPos.first + 1, file.lines[cursorPos.first].substr(cursorPos.second));
				file.lines[cursorPos.first].erase(cursorPos.second);
				file.charType[cursorPos.first].erase(
					file.charType[cursorPos.first].begin() + cursorPos.second,
					file.charType[cursorPos.first].end()
				);
			}
			else{
				file.insert(cursorPos.first + 1, "");
			}
			vector<int> ct = calcCharType(lns[0]);
			file.lines[cursorPos.first].insert(
				file.lines[cursorPos.first].end(),
				lns[0].begin(),
				lns[0].end()
			);
			file.charType[cursorPos.first].insert(
				file.charType[cursorPos.first].end(),
				ct.begin(),
				ct.end()
			);
			
			/**********************************************\
			                   MID LINE
			\**********************************************/
			int endIdx = cursorPos.first + 1;
			for(int i = 1; i + 1 < lns.size(); i++){
				file.insert(endIdx++, lns[i]);
			}
			
			/**********************************************\
			                  LAST LINE
			\**********************************************/
			const string& e = lns[lns.size() - 1];
			ct = calcCharType(e);
			file.lines[endIdx].insert(
				file.lines[endIdx].begin(),
				e.begin(),
				e.end()
			);
			file.charType[endIdx].insert(
				file.charType[endIdx].begin(),
				ct.begin(),
				ct.end()
			);
			
			cursorPos = make_pair(endIdx, e.length());
		}
	}
	void selectAllCallback(){
		selectStart = make_pair(0, 0);
		selectEnd = make_pair(file.lines.size() - 1, static_cast<int>(file.lines[file.lines.size() - 1].size()) - 1);
	}
	void saveCallback(){
		curPage.savePageWithCurPath();
		if(isServer) alert(0x0A, "保存成功"); 
	}
	void refreshCallback(){
		curPage.getPageWithCurPath();
		scroll = make_pair(0, 0);
		cursorPos = make_pair(0, 0);
	}
	void uScrollCallback(){
		scroll.first = max(0, scroll.first - 3);
	}
	void dScrollCallback(){
		scroll.first = min((int)file.lines.size() - 1, scroll.first + 3);
	}
	void lScrollCallback(){
		scroll.second = max(0, scroll.second - 4);
	}
	void rScrollCallback(){
		scroll.second += 4;
	}
	void setupFunctions(){
//		wlog("DEBUG", "setupFunctions()");
		
		for(int i=0;i<btns.size();i++){
			btns[i].setHoverCallback(
				[i](){
					btns[i].setCol(0x8F);
				}
			);
			btns[i].setUnhoverCallback(
				[i](){
					btns[i].setCol(0x0F);
				}
			);
		}
		
		btns[0].setClickCallback(cutCallback);
		btns[1].setClickCallback(copyCallback);
		btns[2].setClickCallback(pasteCallback); 
		btns[3].setClickCallback(selectAllCallback);
		btns[4].setClickCallback(saveCallback);
		btns[5].setClickCallback(refreshCallback);
		btns[6].setDragCallback(uScrollCallback);
		btns[7].setDragCallback(dScrollCallback);
		btns[8].setDragCallback(lScrollCallback);
		btns[9].setDragCallback(rScrollCallback);
	} 
	void outputFunctions(){
		tOutput(string(120, '='), 28, 0);
		for(int i=0;i<btns.size();i++){
			btns[i].process();
		}
		stringstream status;
		status << "行：" << cursorPos.first + 1 << "/" << file.lines.size() 
		       << "  列：" << cursorPos.second + 1 << "/" << file.lines[cursorPos.first].size() + 1;
		tOutput(status.str(), 29, 76);
	}
	void processNormalPage(){
//		wlog("DEBUG", "call processNormalPage()");
		setupMouseInfo();
		processMouseInput();
		clampPos();
		processInput();
		clampPos();
		tEnable(T_SCISSOR_TEST);
		tScissor(2, 0, 26, 120);
		outputCursor();
		outputSelect();
		tSetOutputCol(0x0F);
		outputText();
		tDisable(T_SCISSOR_TEST);
		outputFunctions();
		processAlert();
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
		if(focus == 0 && tKbhit()){
			wchar_t wch = tGetwch();
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
				if(curPage.type == 0) curPage.savePageWithCurPath();
				if(url.length() <= urlHeadLength){
					curPage.type = 404;
				} 
				else{
					curPage.path = url.substr(urlHeadLength);
					curPage.getPageWithCurPath();
				}
			}
			else if(wch == 224){
				wch = tGetwch();
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
