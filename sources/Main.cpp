﻿// © 2017 Naoki Nakagawa
#include <fstream>
#include <string>
#include <regex>
#include <crtdbg.h>
#include "Game.h"

using namespace std;
using namespace DirectX;
using namespace GameLibrary;

string GetSourceCode(const char* filePath) {
	ifstream sourceFile(filePath);
	istreambuf_iterator<char> iterator(sourceFile);
	istreambuf_iterator<char> last;
	string sourceCode(iterator, last);
	sourceFile.close();
	return sourceCode;
}

void MargeSourceCode(const char* file, string& sourceCode) {
	string from = "#include \"" + string(file) + "\"";
	string filePath = "sources/" + string(file);

	string::size_type pos = sourceCode.find(from);
	sourceCode.replace(pos, from.size(), GetSourceCode(filePath.c_str()));
}

void LinkLibrary() {
	string game = GetSourceCode("sources/Game.h");

	//MargeSourceCode("Material.h", game);
	MargeSourceCode("Sprite.h", game);
	MargeSourceCode("Text.h", game);

	ofstream libraryFile("sources/GameLibrary.h");
	libraryFile << game;
	libraryFile.close();
}

int main() {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	LinkLibrary();

	Sprite sprite1(L"clock.png");
	sprite1.position = XMFLOAT2(200.0f, 200.0f);
	Sprite sprite2(L"4.JPG");
	sprite2.position = XMFLOAT2(1000.0f, 250.0f);

	Game::AddFont(L"衡山毛筆フォント行書.ttf");
	Game::SetTitle(L"くぁwせdrftgyふじこlp");

	Text text1(L"あ");
	text1.scale.x = text1.scale.y = 0.5f;
	Text text2(L"あ", L"ＭＳ 明朝");
	Text text3(L"玉", L"衡山毛筆フォント行書");

	bool isFullscreen = false;
	float t = 0.0f;
	while (Game::Loop()) {
		t += Game::GetDeltaTime();

		sprite1.angle = t * (360.0f);
		sprite1.scale.y = 1.3f;
		sprite1.Draw();
		sprite1.angle = t * (360.0f / 60.0f);
		sprite1.scale.y = 1.0f;
		sprite1.Draw();
		sprite1.angle = t * (360.0f / 60.0f / 12.0f);
		sprite1.scale.y = 0.7f;
		sprite1.Draw();

		sprite2.Draw();

		if (Game::GetKeyDown(VK_RBUTTON)) {
			isFullscreen = !isFullscreen;
			Game::SetFullScreen(isFullscreen);
		}

		if (Game::GetKey(VK_LBUTTON)) {
			text1.position = XMFLOAT2(Game::GetMousePosition().x, Game::GetMousePosition().y);
		}
		text1.angle = Game::GetTime() * -100.0f;
		text1.Draw();

		text2.position = XMFLOAT2(Game::GetSize().x / 2.0f, Game::GetSize().y / 2.0f - 200.0f);
		text2.Draw();

		text3.position = XMFLOAT2(Game::GetSize().x / 2.0f, Game::GetSize().y / 2.0f + 200.0f);
		text3.scale.x = text3.scale.y = 2.0f + sinf(t) * 1.0f;
		text3.Draw();
	}

	return 0;
}
