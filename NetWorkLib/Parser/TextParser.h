#ifndef __PARSER__
#define __PARSER__
#include "pch.h"



// 사용법
// txParser라는 선언 되어 있는 전역 객체를 이용해서 사용
// 
// 1. getdata : 텍스트 파일을 열어서 버퍼에 보관해주는 함수
// 2. search data : 문자열을 입력하면(key) 그 value를 채워주는 함수 
// 3. close data : 자료를 다 읽어서 저장했으면 버퍼를 삭제해주는 함수
//

class TextParser {
	char* _dataBuf;
	char* _dataPos;
	char* _strBuf;
	int _bufferSize;

public:
	bool GetData(const char* fileName);

	void SkipNoneCommand() {
		while (1) {
			if (*_dataPos == ' ' || *_dataPos == '\n' || *_dataPos == 0x09 || *_dataPos == 0x08)
			{
				_dataPos++;
			}
			else if (*_dataPos == '/' && *(_dataPos + 1) == '/')
			{
				while (*_dataPos != '\n') {
					_dataPos++;
				}

			}
			else if (*_dataPos == '/' && (*_dataPos + 1) == '*') {
				while ((*_dataPos - 1) != '*' || *_dataPos != '/')
				{
					_dataPos++;

					if (_dataPos >= _dataBuf + _bufferSize - 1) {
						// 주석 해제 문자가 없는데 버퍼의 끝에 도달하면 오류로 간주
						printf("주석 처리 오류 \n");
						return;
					}
				}
				_dataPos++;
			}
			else {
				break;
			}
		}
	}


	template <typename T>
	bool SearchData(const char* targetKey, T* targetBuf) {
		//정수일 경우를 상정, char일 경우는 특수화 진행
		_dataPos = _dataBuf; // 처음부터 찾기

		size_t targetLen = strlen(targetKey);
		char* dataKey = new char[targetLen + 1];


		while (1) {
			if (_dataPos >= _dataBuf + _bufferSize - 1) {
				std::cout << "유효한 데이터가 내부에 없습니다" << std::endl;
				delete[] dataKey;
				return false;
			}
			memset(dataKey, 0, targetLen + 1);

			SkipNoneCommand(); //유효한 문자열까지 점프

			int j = 0;
			while (1)
			{
				//문자열을 받아서 저장하고 
				dataKey[j] = *_dataPos;

				_dataPos++;
				j++;

				if (*_dataPos == ' ' || *_dataPos == '\n' || *_dataPos == 0x09 || *_dataPos == 0x08 ||
					j > targetLen - 1 ||
					(*_dataPos == '/' && *(_dataPos + 1) == '/') || (*_dataPos == '/' && *(_dataPos + 1) == '*'))
				{
					break;
				}
			}
			dataKey[targetLen] = NULL; //배열 마지막 값에 널 문자를 넣어주고 
			if (strcmp(dataKey, targetKey) == 0) {
				//맞는 경우 (틀리는 경우는 다시 반복문으로 돌아감
				SkipNoneCommand();
				//각 기호에 맞는 로직
				if (*_dataPos == '=') {
					_dataPos++;
					SkipNoneCommand();

					if (*_dataPos >= 0x30 && *_dataPos <= 0x39) //첫 문자가 정수의 범위에 들어있다면 숫자로 간주
					{
						char numBuf[20];
						int i = 0;
						while (*_dataPos >= 0x30 && *_dataPos <= 0x39)
						{
							numBuf[i] = *_dataPos;
							_dataPos++;
							i++;
						}
						numBuf[i] = NULL;
						*targetBuf = atoi(numBuf);// stoi가 안된다면 다른 방법 시도 근데 왜 안돼

						delete[] dataKey;
						return true;
					}
					else if (*_dataPos >= 0x21 && *_dataPos <= 0x7E) { // 첫 문자가 숫자가 아니라면 문자열로 간주
						int i = 0;
						while (*_dataPos >= 0x21 && *_dataPos <= 0x7E) {
							*(targetBuf + i) = *_dataPos;
							_dataPos++;
							i++;
						}
						*(targetBuf + i) = NULL;
						//버퍼 오버런에 대한 대책은 따로 안 만들었음 

						delete[] dataKey;
						return true;
					}

				}
				//대입 연산자 이외 업데이트를 위한 공간

			}

		}
	}

	void CloseData();

};
extern TextParser txParser;

//txParser을 사용해서 파서 실행



#endif