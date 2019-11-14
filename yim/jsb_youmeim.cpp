//
//  jsb_downloader.cpp
//  cocos2d_js_bindings
//
//  Created by joexie on 16/07/19.
//
//



#include "jsb_youmeim.h"
#include "cocos2d.h"
#include "scripting/js-bindings/manual/jsb_conversions.hpp"
#include "scripting/js-bindings/manual/jsb_global.h"
#include "json/document.h"
#include "json/writer.h"
#include "json/stringbuffer.h"
#include <YIM.h>
#include <mutex>
#include <YIMCInterface.h>
#include <YIMPlatformDefine.h>
#include "base/CCScheduler.h"
#include "platform/CCApplication.h"
#include <queue>
#include <audio/include/AudioEngine.h>
//C:\testjs\MyJSGame\frameworks\cocos2d-x\build
//C:\testjs\MyJSGame\frameworks\cocos2d-x\build
//C:\testjs\MyJSGame\frameworks\cocos2d-x\build\Debug.win32\js-tests\src\tests-main.js
using namespace cocos2d; 


//信号量的一个封装，不知道cocos2dx有木有

#ifdef WIN32
#include <Windows.h>
typedef HANDLE SEMAPHORE_T;
#else
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

#include <chrono> 
#include <thread>
//#include <sys/syslimits.h>
#include <sys/errno.h>

#include <unistd.h>

#	include <fcntl.h> /* O_CREAT */
#	include <sys/stat.h> /* S_IRUSR, S_IWUSR*/
typedef sem_t* SEMAPHORE_T;

#ifndef NAME_MAX
#define NAME_MAX 255

#endif
#	include <fcntl.h> /* O_CREAT */
#	include <sys/stat.h> /* S_IRUSR, S_IWUSR*/
#endif // WIN32

struct ContactsMsgInfo
{
	std::string strContactID;
	YIMMessageBodyType iMessageType;
	std::string strMessageContent;
	unsigned int iCreateTime;

	ContactsMsgInfo() : iMessageType(MessageBodyType_Unknow), iCreateTime(0) {}
};

struct HistoryMsg
{
	int iChatType;
	int iDuration ;
	int iMessageType ;
	unsigned int iCreateTime;
	std::string strParam ;
	std::string strReceiveID ;
	std::string strSenderID ;
	std::string strSerial ;
	std::string strText ;
	std::string strLocalPath;
	HistoryMsg()
	{
		iDuration = 0;
	}
};

struct NearbyLocation
{
	unsigned int distance;
	double longitude;
	double latitude;
	std::string userID;
	std::string country;
	std::string province;
	std::string city;
	std::string districtCounty;
	std::string street;

	NearbyLocation() : distance(0), longitude(0), latitude(0) {}
};

struct InnerForbidInfo {
	std::string channelID;
	int isForbidRoom;
	int  reasonType;
	std::string endTime;
};

//最近联系人信息 to jsval
se::Object* contactsmsgnfo_to_jsval(std::shared_ptr<ContactsMsgInfo> cInfo)
{
	se::Object* obj = se::Object::createPlainObject();

	obj->setProperty("ContactID", se::Value(cInfo->strContactID));
	obj->setProperty("MessageType", se::Value(cInfo->iMessageType));
	obj->setProperty("MessageContent", se::Value(cInfo->strMessageContent));
	obj->setProperty("CreateTime", se::Value(cInfo->iCreateTime));

	return obj;
}

se::Object* forbidinfo_to_jsval(std::shared_ptr<InnerForbidInfo> pInfo)
{

	se::Object* obj = se::Object::createPlainObject();

	obj->setProperty("channelID", se::Value(pInfo->channelID));
	obj->setProperty("isForbidRoom", se::Value(pInfo->isForbidRoom));
	obj->setProperty("reasonType", se::Value(pInfo->reasonType));
	obj->setProperty("endTime", se::Value(pInfo->endTime));

	return obj;
}

se::Object* nearbylocation_to_jsval(std::shared_ptr<NearbyLocation> location)
{
	se::Object* obj = se::Object::createPlainObject();

	obj->setProperty("Distance", se::Value(location->distance));
	obj->setProperty("Longitude", se::Value(location->longitude));
	obj->setProperty("Latitude", se::Value(location->latitude));
	obj->setProperty("UserID", se::Value(location->userID));
	obj->setProperty("Country", se::Value(location->country));
	obj->setProperty("Province", se::Value(location->province));
	obj->setProperty("City", se::Value(location->city));
	obj->setProperty("DistrictCounty", se::Value(location->districtCounty));
	obj->setProperty("Street", se::Value(location->street));

	return obj;
}

se::Object* historymsg_to_jsval(std::shared_ptr<HistoryMsg> pMsg)
{
	se::Object* obj = se::Object::createPlainObject();

	obj->setProperty("ChatType", se::Value(pMsg->iChatType));
	obj->setProperty("Duration", se::Value(pMsg->iDuration));
	obj->setProperty("Param", se::Value(pMsg->strParam));
	obj->setProperty("ReceiveID", se::Value(pMsg->strReceiveID));
	obj->setProperty("SenderID", se::Value(pMsg->strSenderID));
	obj->setProperty("Serial", se::Value(pMsg->strSerial));
	obj->setProperty("Text", se::Value(pMsg->strText));
	obj->setProperty("LocalPath", se::Value(pMsg->strLocalPath));
	obj->setProperty("MessageType", se::Value(pMsg->iMessageType));
	obj->setProperty("CreateTime", se::Value(pMsg->iCreateTime));

	return obj;
}

class CXSemaphore
{
public:
	CXSemaphore(int iInitValue = 0)
	{

#ifdef WIN32
		m_handle = CreateSemaphore(NULL, iInitValue, 0x7fffffff, NULL);
#else
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS) || (CC_TARGET_PLATFORM ==CC_PLATFORM_MAC )
			struct timeval tv;
			gettimeofday(&tv, NULL);
			uint64_t ulEcho = (((uint64_t)tv.tv_sec)*(uint64_t)1000) + (((uint64_t)tv.tv_usec) / (uint64_t)1000);
			snprintf(m_semName, NAME_MAX, "/SEM/%llu/%d", ulEcho, rand() ^ rand());
			m_handle = sem_open(m_semName, O_CREAT, S_IRUSR | S_IWUSR, iInitValue);
			assert(m_handle != SEM_FAILED);
	#else
			m_handle = (SEMAPHORE_T)malloc(sizeof(sem_t));
			sem_init(m_handle, 0, iInitValue);
	#endif
#endif // WIN32
	}	
	~CXSemaphore()
	{

#ifdef WIN32
		CloseHandle(m_handle);
#else
		assert(SEM_FAILED != m_handle);
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS) || (CC_TARGET_PLATFORM ==CC_PLATFORM_MAC )
		if (SEM_FAILED != m_handle) {
			sem_close(m_handle);
		}
#else
		sem_destroy(m_handle);
		free(m_handle);
#endif
#endif // WIN32
	}

	bool Increment()
	{

#ifdef WIN32
		return (bool)ReleaseSemaphore(m_handle, 1, NULL);
#else
		int ret = sem_post(m_handle);
		if (ret == 0)
		{
			return true;
		}
		return false;
#endif // WIN32
	}
	bool Decrement()
	{

#ifdef WIN32
		if (WaitForSingleObject(m_handle, INFINITE) == WAIT_OBJECT_0)
		{
			return true;
	}
		return false;
#else
		int ret = -1;
		do
		{
			ret = sem_wait(m_handle);
		} while (errno == EINTR);
		return ret == 0;
#endif
	}

	SEMAPHORE_T m_handle;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS) || (CC_TARGET_PLATFORM ==CC_PLATFORM_MAC )
	char m_semName[NAME_MAX + 1];

#endif
};

enum WaitResult
{
	WaitResult_Timeout = 1,
	WaitResult_NoTimeout = 2
};

class CXCondWait
{
public:
	CXCondWait()
	{
		m_bIsAlreadySetSignal = false;
	}
	void Reset()
	{
		std::unique_lock <std::mutex> lck(m_mutex);
		m_bIsAlreadySetSignal = false;
	}
	int Wait()
	{
		std::unique_lock <std::mutex> lck(m_mutex);
		if (m_bIsAlreadySetSignal) {
			m_bIsAlreadySetSignal = false;
			return WaitResult_NoTimeout;
		}
		m_condivar.wait(lck);
		m_bIsAlreadySetSignal = false;
		return WaitResult_NoTimeout;
	}

	//0 成功等到，TIMED_OUT 超时， 其他错误
	WaitResult WaitTime(uint64_t msTime)
	{
		std::unique_lock <std::mutex> lck(m_mutex);
		if (m_bIsAlreadySetSignal) {
			m_bIsAlreadySetSignal = false;
			return WaitResult_NoTimeout;
		}
		std::cv_status status = m_condivar.wait_for(lck, std::chrono::milliseconds(msTime));
		m_bIsAlreadySetSignal = false;
		if (status == std::cv_status::timeout) {
			return WaitResult_Timeout;
		}
		return WaitResult_NoTimeout;
	}
	int SetSignal()
	{
		std::unique_lock <std::mutex> lck(m_mutex);
		m_bIsAlreadySetSignal = true;
		m_condivar.notify_one();
		return  0;
	}
private:
	std::mutex m_mutex;
	std::condition_variable m_condivar;
	bool m_bIsAlreadySetSignal;
};

enum YouMeIMCommand
{
	CMD_UNKNOW = 0,
	CMD_LOGIN = 1,
	CMD_HEARTBEAT = 2,
	CMD_LOGOUT = 3,
	CMD_ENTER_ROOM = 4,
	CMD_LEAVE_ROOM = 5,
	CMD_SND_TEXT_MSG = 6,
	CMD_SND_VOICE_MSG = 7,
	CMD_SND_FILE_MSG = 8,
	CMD_GET_MSG = 9,
	CMD_GET_UPLOAD_TOKEN = 10,
	CMD_KICK_OFF = 11,
	CMD_SND_BIN_MSG = 12,
	CMD_RELOGIN = 13,
	CMD_CHECK_ONLINE = 14,
	CMD_SND_GIFT_MSG = 15,
	CMD_GET_ROOM_HISTORY_MSG = 16,
	CMD_GET_USR_INFO = 17,
	CMD_UPDATE_USER_INFO = 18,
	CMD_SND_TIPOFF_MSG = 19,
	CMD_GET_TIPOFF_MSG = 20,
	CMD_GET_DISTRICT = 21,
	CMD_GET_PEOPLE_NEARBY = 22,
	CMD_QUERY_NOTICE = 23,
	CMD_SET_MASK_USER_MSG = 24,
	CMD_GET_MASK_USER_MSG = 25,
	CMD_CLEAN_MASK_USER_MSG = 26,
	CMD_GET_ROOM_INFO = 27,
	CMD_LEAVE_ALL_ROOM = 28,        // 离开所有房间
	CMD_GET_FORBID_RECORD = 31,

	//关系链管理增加
	CMD_REGISTER_USER_PROFILE = 36,
	CMD_QUERY_USER_PROFILE = 37,
	CMD_UPDATE_USER_PROFILE = 38,
	CMD_UPDATE_ONLINE_STATUS = 39,
	CMD_FIND_FRIEND_BY_ID = 40,				// 按ID查找好友
	CMD_FIND_FRIEND_BY_NICKNAME = 41,		// 按昵称查找好友
	CMD_REQUEST_ADD_FRIEND = 42,			// 请求添加好友
	CMD_FRIND_NOTIFY = 44,					// 好友请求通知
	CMD_DELETE_FRIEND = 45,					// 删除好友
	CMD_BLACK_FRIEND = 46,					// 拉黑好友
	CMD_UNBLACK_FRIEND = 47,				// 解除黑名单
	CMD_DEAL_ADD_FRIEND = 48,				// 好友验证
	CMD_QUERY_FRIEND_LIST = 49,				// 获取好友列表
	CMD_QUERY_BLACK_FRIEND_LIST = 50,		// 获取黑名单列表
	CMD_QUERY_FRIEND_REQUEST_LIST = 51,		// 获取好友验证消息列表
	CMD_UPDATE_FRIEND_RQUEST_STATUS = 52,	// 更新好友请求状态
	CMD_RELATION_CHAIN_HEARTBEAT = 53,		// 关系链心跳

	CMD_HXR_USER_INFO_CHANGE_NOTIFY = 74,	// 用户信息变更通知

	//服务器通知
	NOTIFY_LOGIN = 10001,
	NOTIFY_PRIVATE_MSG,
	NOTIFY_ROOM_MSG,

	//客户端(C接口使用)
	CMD_DOWNLOAD = 20001,
	CMD_SEND_MESSAGE_STATUS,
	CMD_RECV_MESSAGE,
	CMD_STOP_AUDIOSPEECH,
	CMD_QUERY_HISTORY_MESSAGE,
	CMD_GET_RENCENT_CONTACTS,
	CMD_RECEIVE_MESSAGE_NITIFY,
	CMD_QUERY_USER_STATUS,
	CMD_AUDIO_PLAY_COMPLETE,
	CMD_STOP_SEND_AUDIO = 20010,
	CMD_TRANSLATE_COMPLETE,
	CMD_DOWNLOAD_URL,
	CMD_GET_MICROPHONE_STATUS,
	CMD_USER_ENTER_ROOM,
	CMD_USER_LEAVE_ROOM,
	CMD_RECV_NOTICE,
	CMD_CANCEL_NOTICE,
	CMD_GET_SPEECH_TEXT,  // 仅需要语音的文字识别内容
	CMD_GET_RECONNECT_RESULT, //重连结果
	CMD_START_RECONNECT = 20020,  // 开始重连
	CMD_RECORD_VOLUME,    // 音量
	CMD_GET_DISTANCE,
	CMD_REQUEST_ADD_FRIEND_NOTIFY = 20023,	// 请求添加好友通知
	CMD_ADD_FRIENT_RESULT_NOTIFY = 20024,	// 添加好友结果通知
	CMD_BE_ADD_FRIENT = 20025,				// 被好友添加通知
	CMD_BE_DELETE_FRIEND_NOTIFY = 20026,	// 被删除好友通知
	CMD_BE_BLACK_FRIEND_NOTIFY = 20027,		// 被拉黑好友通知
	CMD_GET_USER_PROFILE = 20028,			//关系链-查询用户信息
	CMD_SET_USER_PROFILE = 20029,			//关系链-设置用户信息
	CMD_SET_USER_PHOTO = 20030,				//关系链-设置头像
	CMD_SWITCH_USER_STATE = 20031			//关系链-切换用户在线状态
};
#ifdef WIN32

std::wstring P_UTF8_to_Unicode(const char* in, int len)
{
	wchar_t* pBuf = new wchar_t[len + 1];
	if (NULL == pBuf)
	{
		return __XT("");
	}
	size_t out_len = (len + 1) * sizeof(wchar_t);
	memset(pBuf, 0, (len + 1) * sizeof(wchar_t));
	wchar_t* pResult = pBuf;
	out_len = ::MultiByteToWideChar(CP_UTF8, 0, in, len, pBuf, len * sizeof(wchar_t));
	std::wstring out;
	out.assign(pResult, out_len);


	delete[] pResult;
	pResult = NULL;
	return out;
}

std::string P_Unicode_to_UTF8(const wchar_t* in)
{
	std::string out;
	int len = wcslen(in);
	size_t out_len = len * 3 + 1;
	char* pBuf = new char[out_len];
	if (NULL == pBuf)
	{
		return "";
	}
	char* pResult = pBuf;
	memset(pBuf, 0, out_len);


	out_len = ::WideCharToMultiByte(CP_UTF8, 0, in, len, pBuf, len * 3, NULL, NULL);
	out.assign(pResult, out_len);
	delete[] pResult;
	pResult = NULL;
	return out;
}

#define  UTF8TOPlatString(str) P_UTF8_to_Unicode(str.c_str(),str.length())
#define  PlatStringToUTF8(str) P_Unicode_to_UTF8(str.c_str())
#else
#define  UTF8TOPlatString(str) str 
#define  PlatStringToUTF8(str) str
#endif // WIN32
class CYouMeIMJsWrapper;
CYouMeIMJsWrapper* g_IMSingleInstance;

se::Object* __jsb_YouMeIM_proto = nullptr;
se::Class* __jsb_YouMeIM_class = nullptr;

std::string getArrayJsonString(const se::Value& v) {
	std::vector<std::string> lst;
	bool ok = seval_to_std_vector_string(v, &lst);

	if (ok == false) {
		return "";
	}

	rapidjson::Document document;
	rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
	rapidjson::Value root(rapidjson::kArrayType);
	for (int i = 0; i != lst.size(); i++)
	{
		std::string strItem = lst[i];
		rapidjson::Value item;
		item.SetString(strItem.c_str(), strItem.length(), allocator);
		root.PushBack(item, allocator);
	}
	//发送的文本
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	root.Accept(writer);
	std::string reststring = buffer.GetString();

	return reststring;
}

struct AutoPlayInfo
{
	std::string iAudioSerial;
	std::string strSenderID;
	std::string strLocalPath;
	int iDuration;
	AutoPlayInfo()
	{
		iAudioSerial = -1;
		iDuration = 0;
	}
};

class CYouMeIMJsWrapper
{
public:

	
	static XUINT64 str_to_uint64(const char* s)
	{
#if defined(WIN32)
		return _atoi64(s);
#else	
		long long unsigned int out = 0;
		sscanf(s, "%llu", &out);
		return (XUINT64)out;
#endif
	}

	CYouMeIMJsWrapper()
	{
		m_bAutoPlay = false;
		//JSContext* cx = ScriptingCore::getInstance()->getGlobalContext();
		//_JSDelegate.construct(cx);
		m_bIsRecording = false;

	}
	//播放完成回掉
	void PlayFinishCallback(int iAudioID, const std::string & strPath)
	{		
		CCLOG("cocos PlayFinishCallback id: %d ", iAudioID);
		{
			std::lock_guard<std::mutex> lock(m_audioIDMutex);			
			std::map<int, std::string>::iterator it = m_audioIDSenderMap.find(iAudioID);
			if (it != m_audioIDSenderMap.end())
			{
				std::string strSenderid = it->second;
					Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
					CCLOG("cocos notify  OnEndPlay: %d senderid:%s", iAudioID, strSenderid.c_str());
					se::Value func;
					if (_refObj->getProperty("OnEndPlay", &func)) {
						se::ScriptEngine::getInstance()->clearException();
						se::AutoHandleScope hs;

						se::ValueArray args;
						args.push_back(se::Value(strSenderid.c_str()));
						func.toObject()->call(args, _refObj);
					}
				});
				m_audioIDSenderMap.erase(it);
			}
		}
		m_playFinishWait.SetSignal();
		//cocos2d::FileUtils::getInstance()->removeFile(strPath);
	}
	//自动播放线程
	void AutoPlayProc()
	{
		while (true)
		{
			m_playSemaphore.Decrement();
			//从队列中获取一个消息序号
			AutoPlayInfo autoPlayinfo;
			{
				std::lock_guard<std::mutex> lock(m_autoPlayQueueMutex);
				if (m_autoPlayQueue.empty())
				{
					continue;
				}
				autoPlayinfo = m_autoPlayQueue.back();
				m_autoPlayQueue.pop();
			}
			//先判断本地是否有了		
			std::stringstream strFullPathStream;
			if (autoPlayinfo.strLocalPath != "")
			{
				strFullPathStream << autoPlayinfo.strLocalPath;
			}
			else
			{
				std::string strCachePath = cocos2d::FileUtils::getInstance()->getWritablePath();
				strFullPathStream << strCachePath << autoPlayinfo.iAudioSerial << ".wav";
				//先下载到一个临时目录
				IM_DownloadAudioFileSync(str_to_uint64(autoPlayinfo.iAudioSerial.c_str()), UTF8TOPlatString(strFullPathStream.str()).c_str());
			}
			while (m_bIsRecording)
			{
#ifdef WIN32
				Sleep(100);
#else 
				//usleep(100*1000);
				std::this_thread::sleep_for(std::chrono::milliseconds(100)); 
#endif
			
			}

			m_playFinishWait.Reset();

			int iID = cocos2d::AudioEngine::play2d(strFullPathStream.str());
			CCLOG("cocos start auto play ID: %d", iID);
			if (iID != -1)
			{
				//通知开始
				{
					std::lock_guard<std::mutex> lock(m_audioIDMutex);
					m_audioIDSenderMap[iID] = autoPlayinfo.strSenderID;
				}
				
				Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
					se::Value func;
					if (_refObj->getProperty("OnStartPlay", &func)) {
						se::ScriptEngine::getInstance()->clearException();
						se::AutoHandleScope hs;

						se::ValueArray args;
						args.push_back(se::Value(autoPlayinfo.strSenderID.c_str()));
						func.toObject()->call(args, _refObj);
					}
				});
				cocos2d::AudioEngine::setFinishCallback(iID, CC_CALLBACK_2(CYouMeIMJsWrapper::PlayFinishCallback, this));
				CCLOG("cocos set finish callback id: %d,wait finish",iID);
				//最多等待 autoPlayinfo.iDuration+5 秒，如果正常的话 回掉的地方会setsignal
				m_playFinishWait.WaitTime((autoPlayinfo.iDuration + 5) * 1000);

				//需要判断一下是否已经通知过了，如果没有通知的话，还是需要通知一下的
				{
					std::lock_guard<std::mutex> lock(m_audioIDMutex);
					std::map<int, std::string>::iterator it = m_audioIDSenderMap.find(iID);
					if (it != m_audioIDSenderMap.end())
					{
						std::string strSenderid = it->second;
						Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
							CCLOG("cocos notify  OnEndPlay(abnormal): %d senderid:%s", iID, strSenderid.c_str());
							se::Value func;
							if (_refObj->getProperty("OnEndPlay", &func)) {
								se::ScriptEngine::getInstance()->clearException();
								se::AutoHandleScope hs;

								se::ValueArray args;
								args.push_back(se::Value(strSenderid.c_str()));
								func.toObject()->call(args, _refObj);
							}
						});
						m_audioIDSenderMap.erase(it);
					}
				}	
			}		
			//无论成功失败，都需要删掉文件
			cocos2d::FileUtils::getInstance()->removeFile(strFullPathStream.str());			
		}
	}
public:
	se::Object* _refObj;
	bool m_bAutoPlay;
	std::mutex m_autoPlayQueueMutex;
	//播放队列
	std::queue<AutoPlayInfo> m_autoPlayQueue;
	//自动播放线程
	std::thread m_playThread;
	//有语音消息可以播放的信号
	CXSemaphore m_playSemaphore;
	//播放完成的信号
	CXCondWait m_playFinishWait;
	//audio ID sendid 对应表
	std::map<int, std::string> m_audioIDSenderMap;
	std::mutex m_audioIDMutex;
	//是否可以播放的信号，如果当前在录音是不能播放的，默认可以播放
	//std::mutex m_canPlayMute;   
	//多次unlock 会崩溃，暂时先用最简单的 方法来解决
	bool m_bIsRecording;

	void Uninit();
	std::thread m_parseJsonThread;
	bool m_bUninit = false;
};

void CYouMeIMJsWrapper::Uninit()
{
	m_bUninit = true;
	IM_Uninit();
	if (m_parseJsonThread.joinable())
	{
		m_parseJsonThread.join();
	}
}

static bool js_cocos2dx_YouMeIM_finalize(se::State& s) {
	CCLOG("jsbindings: finalizing JS object %p (YouMeIM)", s.thisObject());
	//todo:

	return true;
}
SE_BIND_FINALIZE_FUNC(js_cocos2dx_YouMeIM_finalize)

static bool js_cocos2dx_extension_YouMeIM_constructor(se::State& s)
{
	const auto& args = s.args();
	size_t argc = args.size();
 
	if (argc == 0)
	{
		//assert(g_IMSingleInstance == NULL);
		if (g_IMSingleInstance == NULL)
		{
			g_IMSingleInstance = new CYouMeIMJsWrapper();
		}
		
		// link the native object with the javascript object
		
		g_IMSingleInstance->_refObj = s.thisObject();
		return true;
	}

	SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", argc, 0);
	return false;
}
SE_BIND_CTOR(js_cocos2dx_extension_YouMeIM_constructor, __jsb_YouMeIM_class, js_cocos2dx_YouMeIM_finalize)

void ParseJsonMsgThead()
{
	while (true)
	{
		const XCHAR* pszJsonMsg = IM_GetMessage();
		if (g_IMSingleInstance->m_bUninit)
		{
			break;
		}
		if (pszJsonMsg == NULL)
		{
			continue;
		}
		rapidjson::Document readdoc;
#ifdef WIN32
		readdoc.Parse<0>(P_Unicode_to_UTF8(pszJsonMsg).c_str());
#else
		readdoc.Parse<0>(pszJsonMsg);
#endif
		YouMeIMCommand command = (YouMeIMCommand)readdoc["Command"].GetInt();
		int errorcode = readdoc["Errorcode"].GetInt();
		switch (command) {
		case CMD_RECEIVE_MESSAGE_NITIFY:
		{
			int iChatType = readdoc["ChatType"].GetInt();
			std::string strTargetID = readdoc["TargetID"].GetString();
			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnRecvMessageNotify", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;

					se::ValueArray args;
					args.push_back(se::Value(iChatType));
					args.push_back(se::Value(strTargetID));
					func.toObject()->call(args, g_IMSingleInstance->_refObj);
				}
			});
		}
		break;
		case CMD_GET_RENCENT_CONTACTS:
		{
			rapidjson::Value &contactList = readdoc["contacts"];
			std::vector<std::shared_ptr<ContactsMsgInfo> > contactsMsgInfoList;
			for (int i = 0; i != contactList.Size(); i++)
			{
				std::shared_ptr<ContactsMsgInfo> cInfo(new ContactsMsgInfo);
				const rapidjson::Value& info = contactList[i];
				cInfo->strContactID = info["ContactID"].GetString();
				cInfo->iMessageType = (YIMMessageBodyType)info["MessageType"].GetUint();
				cInfo->strMessageContent = info["MessageContent"].GetString();
				cInfo->iCreateTime = info["CreateTime"].GetUint();

				contactsMsgInfoList.push_back(cInfo);
			}
			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnGetContactList", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;

					se::ValueArray args;
					args.push_back(se::Value(errorcode));

					if (contactsMsgInfoList.size() > 0) {
						se::HandleObject arr(se::Object::createArrayObject(contactsMsgInfoList.size()));
						for (int i = 0; i != contactsMsgInfoList.size(); i++)
						{
							se::HandleObject obj(contactsmsgnfo_to_jsval(contactsMsgInfoList[i]));
							if (obj.isEmpty())
								continue;
							if (!arr->setArrayElement(i, se::Value(obj)))
								break;
						}
						args.push_back(se::Value(arr));
					}

					func.toObject()->call(args, g_IMSingleInstance->_refObj);
				}
			});
		}
		break;
		case CMD_GET_ROOM_HISTORY_MSG:
		{
			int iRemainCount = readdoc["Remain"].GetInt();
			std::string strRoomID = readdoc["RoomID"].GetString();
			rapidjson::Value &msgList = readdoc["MessageList"];
			std::vector<std::shared_ptr<HistoryMsg> >hisoryLists;
			for (int i = 0; i != msgList.Size(); i++)
			{
				std::shared_ptr<HistoryMsg> pMsg(new HistoryMsg);
				const rapidjson::Value& msg = msgList[i];
				pMsg->iMessageType = msg["MessageType"].GetInt();
				pMsg->iChatType = msg["ChatType"].GetInt();
				pMsg->strReceiveID = msg["ReceiveID"].GetString();
				pMsg->strSenderID = msg["SenderID"].GetString();
				pMsg->strSerial = msg["Serial"].GetString();

				pMsg->iCreateTime = msg["CreateTime"].GetUint();

				if (pMsg->iMessageType == MessageBodyType_TXT)
				{
					pMsg->strText = msg["Content"].GetString();
				}
				else if (pMsg->iMessageType == MessageBodyType_CustomMesssage)
				{
					//base4 编码的数据
					pMsg->strText = msg["Content"].GetString();
				}
				else if (pMsg->iMessageType == MessageBodyType_Voice)
				{
					pMsg->iDuration = msg["Duration"].GetInt();
					pMsg->strParam = msg["Param"].GetString();
					pMsg->strText = msg["Text"].GetString();
				}

				hisoryLists.push_back(pMsg);
			}
			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnQueryRoomHistoryMessageFromServer", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;

					se::ValueArray args;
					args.push_back(se::Value(strRoomID));
					args.push_back(se::Value(iRemainCount));

					if (hisoryLists.size() > 0) {
						se::HandleObject arr(se::Object::createArrayObject(hisoryLists.size()));
						for (int i = 0; i != hisoryLists.size(); i++)
						{
							se::HandleObject obj(historymsg_to_jsval(hisoryLists[i]));
							if (obj.isEmpty())
								continue;
							if (!arr->setArrayElement(i, se::Value(obj)))
								break;
						}
						args.push_back(se::Value(arr));
					}

					func.toObject()->call(args, g_IMSingleInstance->_refObj);
				}
			});
		}
		break;
		case CMD_GET_USR_INFO:
		{
			std::string strUserID = readdoc["UserID"].GetString();
			std::string strUserInfo = readdoc["UserInfo"].GetString();

			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnGetUserInfo", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;

					se::ValueArray args;
					args.push_back(se::Value(errorcode));
					args.push_back(se::Value(strUserID));
					args.push_back(se::Value(strUserInfo));
					func.toObject()->call(args, g_IMSingleInstance->_refObj);
				}
			});
		}
		break;
		case CMD_QUERY_HISTORY_MESSAGE:
		{
			int iRemainCount = readdoc["Remain"].GetInt();
			std::string strTargetID = readdoc["TargetID"].GetString();
			rapidjson::Value &msgList = readdoc["messageList"];
			std::vector<std::shared_ptr<HistoryMsg> >hisoryLists;
			for (int i = 0; i != msgList.Size(); i++)
			{
				std::shared_ptr<HistoryMsg> pMsg(new HistoryMsg);
				const rapidjson::Value& msg = msgList[i];
				pMsg->iMessageType = msg["MessageType"].GetInt();
				pMsg->iChatType = msg["ChatType"].GetInt();
				pMsg->strReceiveID = msg["ReceiveID"].GetString();
				pMsg->strSenderID = msg["SenderID"].GetString();
				pMsg->strSerial = msg["Serial"].GetString();
				pMsg->iCreateTime = msg["CreateTime"].GetUint();
				if (pMsg->iMessageType == MessageBodyType_TXT)
				{
					pMsg->strText = msg["Content"].GetString();
				}
				else if (pMsg->iMessageType == MessageBodyType_CustomMesssage)
				{
					//base4 编码的数据
					pMsg->strText = msg["Content"].GetString();
				}
				else if (pMsg->iMessageType == MessageBodyType_Voice)
				{
					pMsg->iDuration = msg["Duration"].GetInt();
					pMsg->strParam = msg["Param"].GetString();
					pMsg->strText = msg["Text"].GetString();
					pMsg->strLocalPath = msg["LocalPath"].GetString();
				}

				hisoryLists.push_back(pMsg);
			}
			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnQueryHistoryMsg", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;

					se::ValueArray args;
					args.push_back(se::Value(strTargetID));
					args.push_back(se::Value(iRemainCount));

					if (hisoryLists.size() > 0) {
						se::HandleObject arr(se::Object::createArrayObject(hisoryLists.size()));
						for (int i = 0; i != hisoryLists.size(); i++)
						{
							se::HandleObject obj(historymsg_to_jsval(hisoryLists[i]));
							if (obj.isEmpty())
								continue;
							if (!arr->setArrayElement(i, se::Value(obj)))
								break;
						}
						args.push_back(se::Value(arr));
					}

					func.toObject()->call(args, g_IMSingleInstance->_refObj);
				}
			});
		}
		break;
		case CMD_STOP_AUDIOSPEECH:
		{
			std::string strDownloadURL = readdoc["DownloadURL"].GetString();
			int iDuration = readdoc["Duration"].GetInt();
			int iFileSize = readdoc["FileSize"].GetInt();
			std::string strLocalPath = readdoc["LocalPath"].GetString();
			std::string ulRequestID = readdoc["RequestID"].GetString();
			std::string strText = readdoc["Text"].GetString();
			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnAudioSpeech", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;

					se::ValueArray args;
					args.push_back(se::Value(ulRequestID));
					args.push_back(se::Value(errorcode));
					args.push_back(se::Value(strDownloadURL));
					args.push_back(se::Value(iDuration));
					args.push_back(se::Value(iFileSize));
					args.push_back(se::Value(strLocalPath));
					args.push_back(se::Value(strText));

					func.toObject()->call(args, g_IMSingleInstance->_refObj);
				}
			});
		}
		break;
		case CMD_DOWNLOAD_URL:
		{
			std::string strFromUrl = readdoc["FromUrl"].GetString();
			std::string strSavePath = readdoc["SavePath"].GetString();

			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnDownloadByUrl", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;

					se::ValueArray args;
					args.push_back(se::Value(errorcode));
					args.push_back(se::Value(strFromUrl));
					args.push_back(se::Value(strSavePath));
					func.toObject()->call(args, g_IMSingleInstance->_refObj);
				}
			});
		}
		break;
		case CMD_DOWNLOAD:
		{
			std::string strSavePath = readdoc["SavePath"].GetString();
			YIMMessageBodyType bodyType = (YIMMessageBodyType)readdoc["MessageType"].GetInt();
			YIMChatType ChatType = (YIMChatType)readdoc["ChatType"].GetInt();
			std::string iSerial = readdoc["Serial"].GetString();
			int iCreateTime = readdoc["CreateTime"].GetInt();
			std::string RecvID = readdoc["ReceiveID"].GetString();
			std::string SenderID = readdoc["SenderID"].GetString();
			std::string Content;
			std::string Param;

			int iDuration = 0;
			if (bodyType == MessageBodyType_Voice)
			{
				Content = readdoc["Text"].GetString();
				Param = readdoc["Param"].GetString();
				iDuration = readdoc["Duration"].GetInt();

				CCLOG("recv voice message senderid:%s", SenderID.c_str());
			}
			else if (bodyType == MessageBodyType_File) {
				//
			}

			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnDownload", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;

					se::ValueArray args;
					args.push_back(se::Value(errorcode));
					args.push_back(se::Value(strSavePath));
					args.push_back(se::Value(bodyType));
					args.push_back(se::Value(ChatType));
					args.push_back(se::Value(iSerial));
					args.push_back(se::Value(RecvID));
					args.push_back(se::Value(SenderID));
					args.push_back(se::Value(Content));
					args.push_back(se::Value(Param));
					args.push_back(se::Value(iDuration));
					args.push_back(se::Value(iCreateTime));

					func.toObject()->call(args, g_IMSingleInstance->_refObj);
				}
			});
		}
		break;
		case CMD_LOGIN:
		{ 
			std::string strYouMeID = readdoc["UserID"].GetString();
			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnLogin", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;
					se::ValueArray args;
					args.push_back(se::Value(errorcode));
					args.push_back(se::Value(strYouMeID));

					func.toObject()->call(args, g_IMSingleInstance->_refObj);
				}
			});
		}
		break;
		case CMD_LOGOUT:
		{
			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnLogout", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;

					func.toObject()->call(se::EmptyValueArray, g_IMSingleInstance->_refObj);
				}
			});
		}
		break;
		case CMD_KICK_OFF:
		{
			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnKickOff", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;

					func.toObject()->call(se::EmptyValueArray, g_IMSingleInstance->_refObj);
				}
			});
		}
		break;

		case CMD_SEND_MESSAGE_STATUS:
		{
			std::string iSerial = readdoc["RequestID"].GetString();
			int iSendTime = readdoc["SendTime"].GetUint();
			int isForbidRoom = readdoc["IsForbidRoom"].GetInt();
			int reasonType = readdoc["reasonType"].GetInt();
			std::string endTime = readdoc["forbidEndTime"].GetString();
			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnSendMessageStatus", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;

					se::ValueArray args;
					args.push_back(se::Value(iSerial));
					args.push_back(se::Value(errorcode));
					args.push_back(se::Value(iSendTime));
					args.push_back(se::Value(isForbidRoom));
					args.push_back(se::Value(reasonType));
					args.push_back(se::Value(endTime));

					func.toObject()->call(args, g_IMSingleInstance->_refObj);
				}
			});
		}
		break;
		case CMD_SND_VOICE_MSG:
		{
			std::string iSerial = readdoc["RequestID"].GetString();
			std::string strText = readdoc["Text"].GetString();
			std::string strLocalPath = readdoc["LocalPath"].GetString();
			int iDuration = readdoc["Duration"].GetInt();
			int iSendTime = readdoc["SendTime"].GetUint();
			int isForbidRoom = readdoc["IsForbidRoom"].GetInt();
			int reasonType = readdoc["reasonType"].GetInt();
			std::string endTime = readdoc["forbidEndTime"].GetString();
			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnSendAudioMessageStatus", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;


					se::ValueArray args;
					args.push_back(se::Value(iSerial));
					args.push_back(se::Value(errorcode));
					args.push_back(se::Value(strText));
					args.push_back(se::Value(strLocalPath));
					args.push_back(se::Value(iDuration));
					args.push_back(se::Value(iSendTime));
					args.push_back(se::Value(isForbidRoom));
					args.push_back(se::Value(reasonType));
					args.push_back(se::Value(endTime));

					//把自己录音的消息也房间队列
					if (g_IMSingleInstance->m_bAutoPlay)
					{
						std::lock_guard<std::mutex> lock(g_IMSingleInstance->m_autoPlayQueueMutex);
						AutoPlayInfo info;
						info.strLocalPath = strLocalPath;
						info.iDuration = iDuration;
						g_IMSingleInstance->m_autoPlayQueue.push(info);
						g_IMSingleInstance->m_playSemaphore.Increment();
					}

					func.toObject()->call(args, g_IMSingleInstance->_refObj);
				}
			});
		}
		break;
		case CMD_RECV_MESSAGE:
		{
			YIMMessageBodyType bodyType = (YIMMessageBodyType)readdoc["MessageType"].GetInt();
			YIMChatType ChatType = (YIMChatType)readdoc["ChatType"].GetInt();
			std::string Serial = readdoc["Serial"].GetString();
			int iCreateTime = readdoc["CreateTime"].GetInt();
			std::string RecvID = readdoc["ReceiveID"].GetString();
			std::string SenderID = readdoc["SenderID"].GetString();
			std::string Content;
			std::string Param;
			
			int GiftID =0;
			int GiftCount =0;
			std::string strAnchor;

			int iDuration = 0;
			if ((bodyType == MessageBodyType_TXT) || (bodyType == MessageBodyType_CustomMesssage))
			{
				Content = readdoc["Content"].GetString();
			}
			else if (bodyType == MessageBodyType_Voice)
			{
				Content = readdoc["Text"].GetString();
				Param = readdoc["Param"].GetString();
				iDuration = readdoc["Duration"].GetInt();
				//语音消息需要判断是否自动播放，如果自动播放的话，需要加入一个队列
				if (g_IMSingleInstance->m_bAutoPlay)
				{
					std::lock_guard<std::mutex> lock(g_IMSingleInstance->m_autoPlayQueueMutex);
					AutoPlayInfo info;
					info.iAudioSerial = Serial;
					info.strSenderID = SenderID;
					info.iDuration = iDuration;
					g_IMSingleInstance->m_autoPlayQueue.push(info);
					g_IMSingleInstance->m_playSemaphore.Increment();
				}
				CCLOG("recv voice message senderid:%s", SenderID.c_str());
			}
			else if (bodyType == MessageBodyType_Gift)
			{
				Param = readdoc["Param"].GetString();
				 GiftID = readdoc["GiftID"].GetInt();
				 GiftCount = readdoc["GiftCount"].GetInt();
				strAnchor = readdoc["Anchor"].GetString();
			}
			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnRecvMessage", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;

					se::ValueArray args;
					args.push_back(se::Value((int)bodyType));
					args.push_back(se::Value((int)ChatType));
					args.push_back(se::Value(se::Value(Serial)));
					args.push_back(se::Value(RecvID));
					args.push_back(se::Value(SenderID));
					args.push_back(se::Value(Content));
					args.push_back(se::Value(Param));
					args.push_back(se::Value(iDuration));
					args.push_back(se::Value(iCreateTime));

					args.push_back(se::Value(GiftID));
					args.push_back(se::Value(GiftCount));
					args.push_back(se::Value(strAnchor));

					func.toObject()->call(args, g_IMSingleInstance->_refObj);
				}
			});
		}
		break;
		case CMD_ENTER_ROOM:
		{
			std::string strRoomID = readdoc["GroupID"].GetString();
			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnJoinChatroom", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;

					se::ValueArray args;
					args.push_back(se::Value(errorcode));
					args.push_back(se::Value(strRoomID));

					func.toObject()->call(args, g_IMSingleInstance->_refObj);
				}
			});
		}
		break;
		case CMD_LEAVE_ROOM:
		{
			std::string strRoomID = readdoc["GroupID"].GetString();
			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnLeaveChatroom", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;

					se::ValueArray args;
					args.push_back(se::Value(errorcode));
					args.push_back(se::Value(strRoomID));

					func.toObject()->call(args, g_IMSingleInstance->_refObj);
				}
			});
		}
		break;

		case CMD_USER_ENTER_ROOM:
		{
			std::string strChannelID = readdoc["ChannelID"].GetString();
			std::string strUserID = readdoc["UserID"].GetString();
			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnUserJoinChatRoom", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;

					se::ValueArray args;
					args.push_back(se::Value(strChannelID));
					args.push_back(se::Value(strUserID));

					func.toObject()->call(args, g_IMSingleInstance->_refObj);
				}
			});
		}
		break;
		case CMD_RECORD_VOLUME:
		{
			
			
		}
		
		break;
		
		
		
		case CMD_USER_LEAVE_ROOM:
		{
			std::string strChannelID = readdoc["ChannelID"].GetString();
			std::string strUserID = readdoc["UserID"].GetString();
			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnUserLeaveChatRoom", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;

					se::ValueArray args;
					args.push_back(se::Value(strChannelID));
					args.push_back(se::Value(strUserID));

					func.toObject()->call(args, g_IMSingleInstance->_refObj);
				}
			});
		}
		break;
		case CMD_QUERY_USER_STATUS:
		{
			std::string strUserID = readdoc["UserID"].GetString();
			int status = readdoc["Status"].GetInt();

			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnQueryUserStatus", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;

					se::ValueArray args;
					args.push_back(se::Value(errorcode));
					args.push_back(se::Value(strUserID));
					args.push_back(se::Value(status));

					func.toObject()->call(args, g_IMSingleInstance->_refObj);
				}
			});
		}
		break;
		case CMD_AUDIO_PLAY_COMPLETE:
		{
			std::string strPath = readdoc["Path"].GetString();
			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnPlayCompletion", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;

					se::ValueArray args;
					args.push_back(se::Value(errorcode));
					args.push_back(se::Value(strPath));

					func.toObject()->call(args, g_IMSingleInstance->_refObj);
				}
			});
		}
		break;
		case CMD_STOP_SEND_AUDIO:
		{
			std::string strText = readdoc["Text"].GetString();
			std::string strLocalPath = readdoc["LocalPath"].GetString();
			int iDuration = readdoc["Duration"].GetInt();
			std::string ulRequestID = readdoc["RequestID"].GetString();

			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnStartSendAudioMessage", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;

					se::ValueArray args;
					args.push_back(se::Value(ulRequestID));
					args.push_back(se::Value(errorcode));
					args.push_back(se::Value(strText));
					args.push_back(se::Value(strLocalPath));
					args.push_back(se::Value(iDuration));

					func.toObject()->call(args, g_IMSingleInstance->_refObj);
				}
			});

		}
		break;
		case CMD_TRANSLATE_COMPLETE:
		{
			unsigned int requestID = readdoc["RequestID"].GetUint();
			std::string text = readdoc["Text"].GetString();
			int srcLangCode = readdoc["SrcLangCode"].GetInt();
			int destLangCode = readdoc["DestLangCode"].GetInt();
			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnTranslateTextComplete", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;

					se::ValueArray args;
					args.push_back(se::Value(errorcode));
					args.push_back(se::Value(requestID));
					args.push_back(se::Value(text));
					args.push_back(se::Value(srcLangCode));
					args.push_back(se::Value(destLangCode));

					func.toObject()->call(args, g_IMSingleInstance->_refObj);
				}
			});
		}
		break;
		case CMD_GET_DISTRICT:
		{
			//YIMErrorcode errorcode = (YIMErrorcode)readdoc["Errorcode"].GetInt();
			unsigned int iDistrictCode = readdoc["DistrictCode"].GetUint();
			std::string strCountry = readdoc["Country"].GetString();
			std::string strProvince = readdoc["Province"].GetString();
			std::string strCity = readdoc["City"].GetString();
			std::string strDistrictCounty = readdoc["DistrictCounty"].GetString();
			std::string strStreet = readdoc["Street"].GetString();
			double fLongitude = readdoc["Longitude"].GetDouble();
			double fLatitude = readdoc["Latitude"].GetDouble();

			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnUpdateLocation", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;

					se::ValueArray args;
					args.push_back(se::Value(iDistrictCode));
					args.push_back(se::Value(strCountry));
					args.push_back(se::Value(strProvince));
					args.push_back(se::Value(strCity));
					args.push_back(se::Value(strDistrictCounty));
					args.push_back(se::Value(strStreet));
					args.push_back(se::Value(fLongitude));
					args.push_back(se::Value(fLatitude));

					func.toObject()->call(args, g_IMSingleInstance->_refObj);
				}

			});
		}
		break;
		case CMD_GET_PEOPLE_NEARBY:
		{
			YIMErrorcode errorcode = (YIMErrorcode)readdoc["Errorcode"].GetInt();
			unsigned int iStartDistance = readdoc["StartDistance"].GetUint();
			unsigned int iEndDistance = readdoc["EndDistance"].GetUint();
			rapidjson::Value &neighbourList = readdoc["NeighbourList"];
			std::vector<std::shared_ptr<NearbyLocation> > nearbyPeopleList;
			for (int i = 0; i != neighbourList.Size(); i++)
			{
				std::shared_ptr<NearbyLocation> pNearbyLocation(new NearbyLocation);
				const rapidjson::Value& location = neighbourList[i];
				pNearbyLocation->userID = location["UserID"].GetString();
				pNearbyLocation->distance = location["Distance"].GetUint();
				pNearbyLocation->longitude = location["Longitude"].GetDouble();
				pNearbyLocation->latitude = location["Latitude"].GetDouble();
				pNearbyLocation->country = location["Country"].GetString();
				pNearbyLocation->province = location["Province"].GetString();
				pNearbyLocation->city = location["City"].GetString();
				pNearbyLocation->districtCounty = location["DistrictCounty"].GetString();
				pNearbyLocation->street = location["Street"].GetString();
				nearbyPeopleList.push_back(pNearbyLocation);
			}
			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnGetNearbyObjects", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;

					se::ValueArray args;
					args.push_back(se::Value(errorcode));
					args.push_back(se::Value(iStartDistance));
					args.push_back(se::Value(iEndDistance));

					if (nearbyPeopleList.size() > 0) {
						se::HandleObject arr(se::Object::createArrayObject(nearbyPeopleList.size()));
						for (int i = 0; i != nearbyPeopleList.size(); i++)
						{
							se::HandleObject obj(nearbylocation_to_jsval(nearbyPeopleList[i]));
							if (obj.isEmpty())
								continue;
							if (!arr->setArrayElement(i, se::Value(obj)))
								break;
						}
						args.push_back(se::Value(arr));
					}

					func.toObject()->call(args, g_IMSingleInstance->_refObj);
				}
			});
		}
		break;
		case CMD_GET_MICROPHONE_STATUS:
		{
			int status = readdoc["Status"].GetInt();
			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnGetMicrophoneStatus", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;

					se::ValueArray args;
					args.push_back(se::Value(status));

					func.toObject()->call(args, g_IMSingleInstance->_refObj);
				}
			});
		}
		break;
		case CMD_GET_TIPOFF_MSG:
		{
			int iResult = readdoc["Result"].GetInt();
			std::string strUserID = readdoc["UserID"].GetString();
			unsigned int iAccusationTime = readdoc["AccusationTime"].GetUint();
			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnAccusationResultNotify", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;

					se::ValueArray args;
					args.push_back(se::Value(iResult));
					args.push_back(se::Value(strUserID));
					args.push_back(se::Value(iAccusationTime));

					func.toObject()->call(args, g_IMSingleInstance->_refObj);
				}
			});
		}
		break;
		case CMD_RECV_NOTICE:
		{
			std::string ullNoticeID = readdoc["NoticeID"].GetString();
			std::string strChannelID = readdoc["ChannelID"].GetString();
			int iNoticeType = readdoc["NoticeType"].GetInt();
			std::string strContent = readdoc["NoticeContent"].GetString();
			std::string strLinkText = readdoc["LinkText"].GetString();
			std::string strLinkAddress = readdoc["LinkAddress"].GetString();
			unsigned int iBeginTime = readdoc["BeginTime"].GetUint();
			unsigned int iEndTime = readdoc["EndTime"].GetUint();

			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnRecvNotice", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;

					se::ValueArray args;
					args.push_back(se::Value(ullNoticeID));
					args.push_back(se::Value(strChannelID));
					args.push_back(se::Value(iNoticeType));
					args.push_back(se::Value(strContent));
					args.push_back(se::Value(strLinkText));
					args.push_back(se::Value(strLinkAddress));
					args.push_back(se::Value(iBeginTime));
					args.push_back(se::Value(iEndTime));

					func.toObject()->call(args, g_IMSingleInstance->_refObj);
				}
			});
		}
		break;
		case CMD_CANCEL_NOTICE:
		{
			std::string ullNoticeID = readdoc["NoticeID"].GetString();
			std::string strChannelID = readdoc["ChannelID"].GetString();

			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnCancelNotice", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;

					se::ValueArray args;
					args.push_back(se::Value(ullNoticeID));
					args.push_back(se::Value(strChannelID));

					func.toObject()->call(args, g_IMSingleInstance->_refObj);
				}
			});
		}
		break;
		case CMD_GET_FORBIDDEN_INFO:
		{
			YIMErrorcode errorcode = (YIMErrorcode)readdoc["Errorcode"].GetInt();
			rapidjson::Value &forbidList = readdoc["ForbiddenSpeakList"];
			std::vector<std::shared_ptr<InnerForbidInfo> > forbidInfoList;
			for (int i = 0; i != forbidList.Size(); i++)
			{
				std::shared_ptr<InnerForbidInfo> pInfo(new InnerForbidInfo);
				const rapidjson::Value& info = forbidList[i];
				pInfo->channelID = info["ChannelID"].GetString();
				pInfo->isForbidRoom = info["IsForbidRoom"].GetInt();
				pInfo->reasonType = info["reasonType"].GetInt();
				pInfo->endTime = info["forbidEndTime"].GetString();

				forbidInfoList.push_back(pInfo);
			}

			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnGetForbiddenSpeakInfo", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;

					se::ValueArray args;
					args.push_back(se::Value(errorcode));

					if (forbidInfoList.size() > 0) {
						se::HandleObject arr(se::Object::createArrayObject(forbidInfoList.size()));
						for (int i = 0; i != forbidInfoList.size(); i++)
						{
							se::HandleObject obj(forbidinfo_to_jsval(forbidInfoList[i]));
							if (obj.isEmpty())
								continue;
							if (!arr->setArrayElement(i, se::Value(obj)))
								break;
						}
						args.push_back(se::Value(arr));
					}

					func.toObject()->call(args, g_IMSingleInstance->_refObj);
				}
			});
		}
		break;
		case CMD_SET_MASK_USER_MSG:
		{
			std::string strUserID = readdoc["UserID"].GetString();
			int iBlock = readdoc["Block"].GetInt();

			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnBlockUser", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;

					se::ValueArray args;
					args.push_back(se::Value(errorcode));
					args.push_back(se::Value(strUserID));
					args.push_back(se::Value(iBlock));

					func.toObject()->call(args, g_IMSingleInstance->_refObj);
				}
			});
		}
		break;
		case CMD_GET_MASK_USER_MSG:
		{
			rapidjson::Value &userList = readdoc["UserList"];
			std::vector<std::string> users;
			for (int i = 0; i != userList.Size(); i++)
			{
				std::string userID = userList[i].GetString();
				users.push_back(userID);
			}

			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnGetBlockUsers", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;

					se::ValueArray args;
					args.push_back(se::Value(errorcode));
					se::Value lst;
					std_vector_string_to_seval(users, &lst);
					args.push_back(lst);

					func.toObject()->call(args, g_IMSingleInstance->_refObj);
				}
			});
		}
		break;
		case CMD_CLEAN_MASK_USER_MSG:
		{
			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnUnBlockAllUser", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;

					se::ValueArray args;
					args.push_back(se::Value(errorcode));

					func.toObject()->call(args, g_IMSingleInstance->_refObj);
				}
			});
		}
		break;
		case CMD_GET_ROOM_INFO:
		{
			std::string strRoomID = readdoc["RoomID"].GetString();
			unsigned int iCount = readdoc["Count"].GetUint();

			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnGetRoomMemberCount", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;

					se::ValueArray args;
					args.push_back(se::Value(errorcode));
					args.push_back(se::Value(strRoomID));
					args.push_back(se::Value(iCount));

					func.toObject()->call(args, g_IMSingleInstance->_refObj);
				}
			});
		}
		break;
		case CMD_GET_SPEECH_TEXT:
		{
			std::string iSerial = readdoc["RequestID"].GetString();
			std::string strText = readdoc["Text"].GetString();

			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnGetRecognizeSpeechText", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;

					se::ValueArray args;
					args.push_back(se::Value(iSerial));
					args.push_back(se::Value(errorcode));
					args.push_back(se::Value(strText));

					func.toObject()->call(args, g_IMSingleInstance->_refObj);
				}
			});
		}
		break;
		case CMD_GET_RECONNECT_RESULT:
		{
			int iResult = readdoc["Result"].GetInt();

			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnRecvReconnectResult", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;

					se::ValueArray args;
					args.push_back(se::Value(iResult));

					func.toObject()->call(args, g_IMSingleInstance->_refObj);
				}
			});
		}
		break;
		case CMD_START_RECONNECT:
		{
			Application::getInstance()->getScheduler()->performFunctionInCocosThread([=] {
				se::Value func;
				if (g_IMSingleInstance->_refObj->getProperty("OnStartReconnect", &func)) {
					se::ScriptEngine::getInstance()->clearException();
					se::AutoHandleScope hs;

					func.toObject()->call(se::EmptyValueArray, g_IMSingleInstance->_refObj);
				}
			});
		}
		break;

		default:
			break;
		}

		IM_PopMessage(pszJsonMsg);
	}
}
//引擎初始化，只需要调用一次
static bool js_cocos2dx_extension_IM_Init(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 2)
	{

		std::string strAppKey;
		seval_to_std_string(args[0], &strAppKey);
		std::string strAppSecuret;
		seval_to_std_string(args[1], &strAppSecuret);
		//调用SDK 初始化函数
		int iErrorCode = IM_Init(UTF8TOPlatString(strAppKey).c_str(), UTF8TOPlatString(strAppSecuret).c_str());
		s.rval().setInt32(iErrorCode);
		if (YIMErrorcode_Success == iErrorCode)
		{
			if (!g_IMSingleInstance->m_parseJsonThread.joinable())
			{
				g_IMSingleInstance->m_parseJsonThread = std::thread(ParseJsonMsgThead);
			}
		}
		return true;
	}
	SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", argc, 0);
	return false;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_Init)

//引擎反初始化
static bool js_cocos2dx_extension_IM_Uninit(se::State& s) {
	g_IMSingleInstance->Uninit();
	return true;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_Uninit)

static bool js_cocos2dx_extension_IM_OnPause(se::State& s) {
	
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 1)
	{
		int recognition = 1;
		seval_to_int32(args[0], &recognition);

		IM_OnPause(recognition);

		return true;
	}
	SE_REPORT_ERROR( "wrong number of arguments: %d, was expecting %d", argc, 1);
	return false;

}
SE_BIND_FUNC(js_cocos2dx_extension_IM_OnPause)

static bool js_cocos2dx_extension_IM_OnResume(se::State& s) {
	IM_OnResume();
	return true;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_OnResume)

//登陆，登出
static bool js_cocos2dx_extension_IM_Login(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 3)
	{
		
		std::string strYouMeID;
		seval_to_std_string(args[0], &strYouMeID);

		std::string strPasswd;
		seval_to_std_string(args[1], &strPasswd);
		
		std::string strToken;
		seval_to_std_string(args[2], &strToken);

		//调用SDK 初始化函数
		int iErrorCode = IM_Login(UTF8TOPlatString(strYouMeID).c_str(), UTF8TOPlatString(strPasswd).c_str(), UTF8TOPlatString(strToken).c_str());
		s.rval().setInt32(iErrorCode);
		return true;
	}
	SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", argc, 0);
	return false;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_Login)

static bool js_cocos2dx_extension_IM_Logout(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	int iErrorCode = IM_Logout();
	
	s.rval().setInt32(iErrorCode);
	return true;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_Logout)

//消息接口
static bool js_cocos2dx_extension_IM_SendTextMessage(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 3)
	{
		
		std::string strRecvID;
		seval_to_std_string(args[0], &strRecvID);

		int iChatType = 0;
		seval_to_int32( args[1], &iChatType);

		std::string strContent;
		seval_to_std_string(args[2], &strContent);
		std::string pszAttachParam;
		pszAttachParam = "";
		//调用离开聊天室
		XUINT64 iRequestID = -1;
		IM_SendTextMessage(UTF8TOPlatString(strRecvID).c_str(), (YIMChatType)iChatType, UTF8TOPlatString(strContent).c_str(), UTF8TOPlatString(std::string(pszAttachParam)).c_str(), &iRequestID);
		s.rval().setNumber(iRequestID);
		return true;
	}
	SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", argc, 0);
	return false;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_SendTextMessage)

static bool js_cocos2dx_extension_IM_SendCustomMessage(se::State& s) {
	return true;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_SendCustomMessage)

static bool js_cocos2dx_extension_IM_SendOnlyAudioMessage(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 2)
	{
		
		std::string strRecvID;
		seval_to_std_string(args[0], &strRecvID);

		int iChatType = 0;
		seval_to_int32( args[1], &iChatType);

		
		//调用离开聊天室
		XUINT64 iRequestID = -1;
		IM_SendOnlyAudioMessage(UTF8TOPlatString(strRecvID).c_str()
			, (YIMChatType)iChatType, &iRequestID);
		if (-1 != iRequestID)
		{
			//不能播放
			g_IMSingleInstance->m_bIsRecording = true;
		}
		s.rval().setNumber(iRequestID);
		return true;
	}
	SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", argc, 0);
	return false;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_SendOnlyAudioMessage)

static bool js_cocos2dx_extension_IM_SendGift(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 5)
	{
		
		std::string strRecvID;
		seval_to_std_string(args[0], &strRecvID);

		std::string strChannel;
		seval_to_std_string(args[1], &strChannel);

		int iGiftID = 0;
		seval_to_int32( args[2], &iGiftID);

		int iGiftCount = 0;
		seval_to_int32( args[3], &iGiftCount);

		std::string strExtParam;
		seval_to_std_string(args[4], &strExtParam);

		XUINT64 iRequestID = -1;
		IM_SendGift(UTF8TOPlatString(strRecvID).c_str(), UTF8TOPlatString(strChannel).c_str(), iGiftID, iGiftCount,strExtParam.c_str(), &iRequestID);
		s.rval().setNumber(iRequestID);
		return true;
	}
	SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", argc, 0);
	return false;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_SendGift)

static bool js_cocos2dx_extension_IM_GetNewMessage(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 0) {
		return false;
	}
	std::string  strTargets = getArrayJsonString(args[0]);
	int iErrorCode = IM_GetNewMessage(UTF8TOPlatString(strTargets).c_str());
	s.rval().setInt32(iErrorCode);
	return true;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_GetNewMessage)

//获取最近聊天的联系人
static bool js_cocos2dx_extension_IM_GetContact(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	
	int iErrorCode = IM_GetRecentContacts();
	s.rval().setInt32(iErrorCode);
	return true;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_GetContact)

//获取联系人信息
static bool js_cocos2dx_extension_IM_GetUserInfo(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 1) {
		
		std::string strUserID;
		seval_to_std_string(args[0], &strUserID);

		int iErrorCode = IM_GetUserInfo(UTF8TOPlatString(strUserID).c_str());
		s.rval().setInt32(iErrorCode);
		return true;
	}

	SE_REPORT_ERROR( "wrong number of arguments: %d, was expecting %d", argc, 1);
	return false;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_GetUserInfo)

static bool js_cocos2dx_extension_IM_SetUserInfo(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	//传一个json字符串，需要包含nickname,server_area,location,level,vip_level,extra字段
	if (argc == 1) {
		
		std::string userInfo;
		seval_to_std_string(args[0], &userInfo);

		int iErrorCode = IM_SetUserInfo(UTF8TOPlatString(userInfo).c_str());
		s.rval().setInt32(iErrorCode);
		return true;
	}

	SE_REPORT_ERROR( "wrong number of arguments: %d, was expecting %d", argc, 1);
	return false;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_SetUserInfo)

static bool js_cocos2dx_extension_IM_QueryUserStatus(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 1) {
		
		std::string strUserID;
		seval_to_std_string(args[0], &strUserID);

		int iErrorCode = IM_QueryUserStatus(UTF8TOPlatString(strUserID).c_str());
		s.rval().setInt32(iErrorCode);
		return true;
	}

	SE_REPORT_ERROR( "wrong number of arguments: %d, was expecting %d", argc, 1);
	return false;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_QueryUserStatus)

static bool js_cocos2dx_extension_IM_SetVolume(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 1) {
		float volume = 1.0;
		double dp;
		seval_to_double(args[0], &dp);
		volume = (float)dp;

		IM_SetVolume(volume);
		return true;
	}

	SE_REPORT_ERROR( "wrong number of arguments: %d, was expecting %d", argc, 1);
	return false;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_SetVolume)

static bool js_cocos2dx_extension_IM_StartPlayAudio(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 1) {
		

		std::string strPath;
		seval_to_std_string(args[0], &strPath);

		int iErrorCode = IM_StartPlayAudio(UTF8TOPlatString(strPath).c_str());
		s.rval().setInt32(iErrorCode);
		return true;
	}

	SE_REPORT_ERROR( "wrong number of arguments: %d, was expecting %d", argc, 1);
	return false;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_StartPlayAudio)

static bool js_cocos2dx_extension_IM_StopPlayAudio(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 0) {
		

		int iErrorCode = IM_StopPlayAudio();
		s.rval().setInt32(iErrorCode);
		return true;
	}

	SE_REPORT_ERROR( "wrong number of arguments: %d, was expecting %d", argc, 1);
	return false;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_StopPlayAudio)

static bool js_cocos2dx_extension_IM_IsPlaying(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 0) {
		

		bool isPlaying = IM_IsPlaying();
		int iRes = (int)isPlaying;
		s.rval().setInt32(iRes);
		return true;
	}

	SE_REPORT_ERROR( "wrong number of arguments: %d, was expecting %d", argc, 1);
	return false;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_IsPlaying)

static bool js_cocos2dx_extension_IM_GetAudioCachePath(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 0)
	{
		

		//调用SDK 初始化函数
		XCHAR* pszPath = IM_GetAudioCachePath();

#ifdef WIN32
		s.rval().setString(P_Unicode_to_UTF8(pszPath).c_str());
#else
		s.rval().setString(pszPath);
#endif

		IM_DestroyAudioCachePath(pszPath);
		return true;
	}
	SE_REPORT_ERROR( "wrong number of arguments: %d, was expecting %d", argc, 0);
	return false;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_GetAudioCachePath)

static bool js_cocos2dx_extension_IM_ClearAudioCachePath(se::State& s) {
	bool bRes = IM_ClearAudioCachePath();
	int iRes = (int)bRes;
	s.rval().setInt32(iRes);
	return true;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_ClearAudioCachePath)

static bool js_cocos2dx_extension_IM_SetReceiveMessageSwitch(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	
	std::string strTargets = getArrayJsonString(args[0]);

	int iAutoRecv = 1;
	seval_to_int32( args[1], &iAutoRecv);
	
	int iErrorCode = IM_SetReceiveMessageSwitch(UTF8TOPlatString(strTargets).c_str(), iAutoRecv);
	s.rval().setInt32(iErrorCode);
	return true;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_SetReceiveMessageSwitch)

//不关心返回值
static bool js_cocos2dx_extension_IM_MultiSendTextMessage(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 2)
	{
		std::string strTargets = getArrayJsonString(args[0]);
		std::string strText;
		seval_to_std_string(args[1], &strText);

		int iErrorCode = IM_MultiSendTextMessage(strTargets.c_str(),  UTF8TOPlatString(strText).c_str());
		s.rval().setInt32(iErrorCode);
				
		return true;
	}
	SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", argc, 0);
	return false;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_MultiSendTextMessage)


static bool js_cocos2dx_extension_IM_SendAudioMessage(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 2)
	{
		
		//获取两个参数，appkey，appsecrect
		std::string strRecvID;
		seval_to_std_string(args[0], &strRecvID);

		int iChatType = 0;
		seval_to_int32( args[1], &iChatType);
		
		XUINT64 iRequestID = -1;
		IM_SendAudioMessage(UTF8TOPlatString(strRecvID).c_str(), (YIMChatType)iChatType, &iRequestID);
		if (-1 != iRequestID)
		{
			//不能播放
			g_IMSingleInstance->m_bIsRecording = true;
		}
		s.rval().setNumber(iRequestID);
		return true;
	}
	SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", argc, 0);
	return false;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_SendAudioMessage)

static bool js_cocos2dx_extension_IM_StopAudioMessage(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 1)
	{
		
		std::string strParam;
		seval_to_std_string(args[0], &strParam);

		int iErrorCode = IM_StopAudioMessage(UTF8TOPlatString(strParam).c_str());
		g_IMSingleInstance->m_bIsRecording = false;
		s.rval().setInt32(iErrorCode);
		return true;
	}
	return false;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_StopAudioMessage)

static bool js_cocos2dx_extension_IM_CancleAudioMessage(se::State& s) {
	int iErrorCode = IM_CancleAudioMessage();
	
	s.rval().setInt32(iErrorCode);
	g_IMSingleInstance->m_bIsRecording = false;
	return true;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_CancleAudioMessage)

static bool js_cocos2dx_extension_IM_DownloadAudioFile(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 2)
	{
		
		//获取两个参数，appkey，appsecrect
		std::string iSerial;
		seval_to_std_string(args[0], &iSerial);

		std::string strSavePath;
		seval_to_std_string(args[1], &strSavePath);
		//调用离开聊天室

		int iErrorCode = IM_DownloadFile(CYouMeIMJsWrapper::str_to_uint64(iSerial.c_str()), UTF8TOPlatString(strSavePath).c_str(),FileType_Audio);
		s.rval().setInt32(iErrorCode);
		return true;
	}
	SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", argc, 0);
	return false;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_DownloadAudioFile)

static bool js_cocos2dx_extension_IM_DownloadFileByUrl(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 2)
	{
		//获取两个参数，appkey，appsecrect
		std::string strUrl;
		seval_to_std_string(args[0], &strUrl);

		std::string strSavePath;
		seval_to_std_string(args[1], &strSavePath);
		//调用离开聊天室
		int iErrorCode = IM_DownloadFileByURL(UTF8TOPlatString(strUrl).c_str(), UTF8TOPlatString(strSavePath).c_str());
		s.rval().setInt32(iErrorCode);
		return true;
	}
	SE_REPORT_ERROR( "wrong number of arguments: %d, was expecting %d", argc, 0);
	return false;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_DownloadFileByUrl)

//聊天室接口
static bool js_cocos2dx_extension_IM_JoinChatRoom(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 1)
	{
		
		//获取两个参数，appkey，appsecrect
		std::string strID;
		seval_to_std_string(args[0], &strID);

		//调用离开聊天室
		int iErrorCode = IM_JoinChatRoom(UTF8TOPlatString(strID).c_str());
		s.rval().setInt32(iErrorCode);
		return true;
	}
	SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", argc, 0);
	return false;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_JoinChatRoom)

static bool js_cocos2dx_extension_IM_LeaveChatRoom(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 1)
	{
		
		//获取两个参数，appkey，appsecrect
		std::string strID;
		seval_to_std_string(args[0], &strID);

		//调用离开聊天室
		int iErrorCode = IM_LeaveChatRoom(UTF8TOPlatString(strID).c_str());
		s.rval().setInt32(iErrorCode);
		return true;
	}
	SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", argc, 0);
	return false;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_LeaveChatRoom)

static bool js_cocos2dx_extension_IM_SetServerZone(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 1)
	{
		
		int iServerZone = 0;
		seval_to_int32( args[0], &iServerZone);
		IM_SetServerZone((ServerZone)iServerZone);
	}
	return true;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_SetServerZone)

//获取SDK 版本号
static bool js_cocos2dx_extension_IM_GetSDKVer(se::State& s) {
	int iVer  = IM_GetSDKVer();
	s.rval().setInt32(iVer);
	return true;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_GetSDKVer)

static bool js_cocos2dx_extension_IM_DeleteHistoryMessage(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 2)
	{
		int iChatType = 0;
		seval_to_int32(args[0], &iChatType);

		XUINT64 ulTime;
		seval_to_longlong(args[1], (long long*)&ulTime);

		int iErrorCode = IM_DeleteHistoryMessage((YIMChatType)iChatType, ulTime);
		s.rval().setInt32(iErrorCode);
		return true;
	}

	return true;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_DeleteHistoryMessage)

static bool js_cocos2dx_extension_IM_DeleteHistoryMessageByID(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 1)
	{
		
		XUINT64 ulSerialID = 0;
		seval_to_longlong(args[0], (long long *)&ulSerialID);

		int iErrorCode = IM_DeleteHistoryMessageByID(ulSerialID);
		s.rval().setInt32(iErrorCode);
		return true;
	}
	
	return true;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_DeleteHistoryMessageByID)

static bool js_cocos2dx_extension_IM_ConvertAMRToWav(se::State& s){
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 2)
	{
		
		std::string strSrc;
		seval_to_std_string(args[0], &strSrc);

		std::string strDest;
		seval_to_std_string(args[1], &strDest);


		int iErrorCode = IM_ConvertAMRToWav(UTF8TOPlatString(strSrc).c_str(), UTF8TOPlatString(strDest).c_str());
		s.rval().setInt32(iErrorCode);
	}
	return true;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_ConvertAMRToWav)

static bool js_cocos2dx_extension_IM_QueryRoomHistoryMessageFromServer(se::State& s){
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 3)
	{
		
		std::string strRecvID;
		seval_to_std_string(args[0], &strRecvID);
		int iCount = 0;
		seval_to_int32(args[1], &iCount);
		int iDirection = 0;
		seval_to_int32(args[2], &iDirection);
		
		int iErrorCode = IM_QueryRoomHistoryMessageFromServer(UTF8TOPlatString(strRecvID).c_str(), iCount, iDirection);
		s.rval().setInt32(iErrorCode);
	}
	return true;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_QueryRoomHistoryMessageFromServer)

static bool js_cocos2dx_extension_IM_QueryHistoryMessage(se::State& s){
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 5)
	{
		
		std::string strRecvID;
		seval_to_std_string(args[0], &strRecvID);
		int chattype;
		seval_to_int32(args[1], &chattype);
		XUINT64 ulSerialID = 0;
		seval_to_longlong(args[2],(long long *) &ulSerialID);
		int iCount = 0;
		seval_to_int32(args[3], &iCount);
		int iDirection = 0;
		seval_to_int32(args[4], &iDirection);

		int iErrorCode = IM_QueryHistoryMessage(UTF8TOPlatString(strRecvID).c_str(),chattype, ulSerialID,iCount,iDirection);
		s.rval().setInt32(iErrorCode);
	}
	return true;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_QueryHistoryMessage)

static bool js_cocos2dx_extension_IM_StopAudioSpeech(se::State& s){
	int iErrorCode = IM_StopAudioSpeech();
	s.rval().setInt32(iErrorCode);
	return true;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_StopAudioSpeech)

static bool js_cocos2dx_extension_IM_StartAudioSpeech(se::State& s){
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 1)
	{
		
		int bTranslate = 0;
		seval_to_int32( args[0], &bTranslate);
		XUINT64 iRequestID = -1;
		int iErrorCode = IM_StartAudioSpeech(&iRequestID, (bool)bTranslate);
		s.rval().setNumber(iRequestID);
	}
	return true;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_StartAudioSpeech)

static bool js_cocos2dx_extension_IM_SetAutoPlay(se::State& s){
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 1)
	{
		int iAutoPlay = 0;
		seval_to_int32(args[0], &iAutoPlay);
		g_IMSingleInstance->m_bAutoPlay = (bool)iAutoPlay;
		//自动播放
		if (g_IMSingleInstance->m_bAutoPlay)
		{
			//没有启动线程则启动线程
			if (!g_IMSingleInstance->m_playThread.joinable())
			{
				g_IMSingleInstance->m_playThread = std::thread(&CYouMeIMJsWrapper::AutoPlayProc, g_IMSingleInstance);
			}
		}
		else
		{
			std::lock_guard<std::mutex> lock(g_IMSingleInstance->m_autoPlayQueueMutex);
			std::queue<AutoPlayInfo> tmpQueue;
			g_IMSingleInstance->m_autoPlayQueue.swap(tmpQueue);
		}
		return true;
	}
	SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", argc, 0);
	return false;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_SetAutoPlay)

se::Object* filterText_to_jsval(std::string text, int level)
{
	se::Object* obj = se::Object::createPlainObject();
	obj->setProperty("text", se::Value(text));
	obj->setProperty("level", se::Value(level));
	return obj;
}

//关键词过滤
static bool js_cocos2dx_extension_IM_GetFilterText(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 1)
	{
		std::string strText;
		seval_to_std_string(args[0], &strText);

		int level = 0;
		
		//调用SDK 初始化函数
		XCHAR* pszFilteText = IM_GetFilterText(UTF8TOPlatString(strText).c_str(), &level);
		
#ifdef WIN32
		std::string str = P_Unicode_to_UTF8(pszFilteText);
#else
		std::string str = pszFilteText;
#endif
		s.rval().setObject(filterText_to_jsval(str, level));

		IM_DestroyFilterText(pszFilteText);
		return true;
	}
	SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", argc, 0);
	return false;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_GetFilterText)

static bool js_cocos2dx_extension_IM_SetAudioCacheDir(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 1)
	{
		std::string strCacheDir;
		seval_to_std_string(args[0], &strCacheDir);

		//调用离开聊天室
		IM_SetAudioCacheDir(UTF8TOPlatString(strCacheDir).c_str());
		return true;
	}
	SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", argc, 0);
	return false;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_SetAudioCacheDir)

static bool js_cocos2dx_extension_IM_SetRoomHistoryMessageSwitch(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 2)
	{

		std::string strTargets = getArrayJsonString(args[0]);

		int iSave = 1;
		seval_to_int32(args[1], &iSave);

		int iErrorCode = IM_SetRoomHistoryMessageSwitch(UTF8TOPlatString(strTargets).c_str(), iSave);
		s.rval().setInt32(iErrorCode);

		return true;
	}
	SE_REPORT_ERROR( "wrong number of arguments: %d, was expecting %d", argc, 0);
	return false;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_SetRoomHistoryMessageSwitch)

static bool js_cocos2dx_extension_IM_TranslateText(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 3)
	{
		
		std::string strText;
		seval_to_std_string(args[0], &strText);
		int iDestLanguage = 0;
		seval_to_int32(args[1], &iDestLanguage);
		int iSrcLanguage = 0;
		seval_to_int32(args[2], &iSrcLanguage);

		unsigned int iRequestID = -1;
		IM_TranslateText(&iRequestID, UTF8TOPlatString(strText).c_str(), (LanguageCode)iDestLanguage, (LanguageCode)iSrcLanguage);
		s.rval().setUint32(iRequestID);
		return true;
	}
	SE_REPORT_ERROR( "wrong number of arguments: %d, was expecting %d", argc, 0);
	return false;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_TranslateText)

static bool js_cocos2dx_extension_IM_GetCurrentLocation(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	
	int iErrorCode = IM_GetCurrentLocation();
	s.rval().setInt32(iErrorCode);
	return true;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_GetCurrentLocation)

static bool js_cocos2dx_extension_IM_GetNearbyObjects(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 4)
	{
		
		int iCount = 0;
		seval_to_int32(args[0], &iCount);
		std::string strServerAreaID;
		seval_to_std_string(args[1], &strServerAreaID);
		int iDistrictLevel = 0;
		seval_to_int32(args[2], &iDistrictLevel);
		int iResetStartDistance = 0;
		seval_to_int32(args[3], &iResetStartDistance);

		int iErrorCode = IM_GetNearbyObjects(iCount, UTF8TOPlatString(strServerAreaID).c_str(), (DistrictLevel)iDistrictLevel, iResetStartDistance);
		s.rval().setInt32(iErrorCode);
		return true;
	}
	SE_REPORT_ERROR( "wrong number of arguments: %d, was expecting %d", argc, 0);
	return false;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_GetNearbyObjects)

static bool js_cocos2dx_extension_IM_SetUpdateInterval(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 1)
	{
		
		unsigned int iInterval = 0;
		seval_to_uint32(args[0], &iInterval);

		IM_SetUpdateInterval(iInterval);
		return true;
	}
	SE_REPORT_ERROR( "wrong number of arguments: %d, was expecting %d", argc, 0);
	return false;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_SetUpdateInterval)

static bool js_cocos2dx_extension_IM_SetSpeechRecognizeLanguage(se::State& s){
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 1)
	{
		
		int language = 0;
		seval_to_int32(args[0], &language);

		int iErrorCode = IM_SetSpeechRecognizeLanguage(language);
		s.rval().setInt32(iErrorCode);
		return true;
	}
	SE_REPORT_ERROR( "wrong number of arguments: %d, was expecting %d", argc, 0);
	return false;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_SetSpeechRecognizeLanguage)

static bool js_cocos2dx_extension_IM_SetOnlyRecognizeSpeechText(se::State& s){
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 1)
	{
		
		int recognition = 1;
		seval_to_int32(args[0], &recognition);

		int iErrorCode = IM_SetOnlyRecognizeSpeechText(recognition);
		s.rval().setInt32(iErrorCode);
		return true;
	}
	SE_REPORT_ERROR( "wrong number of arguments: %d, was expecting %d", argc, 1);
	return false;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_SetOnlyRecognizeSpeechText)

static bool js_cocos2dx_extension_IM_QueryNotice(se::State& s){
	const auto& args = s.args();
	size_t argc = args.size();
	
	int iErrorCode = IM_QueryNotice();
	s.rval().setInt32(iErrorCode);
	return true;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_QueryNotice)

static bool js_cocos2dx_extension_IM_Accusation(se::State& s){
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 5)
	{
		
		int iSource = 0;
		int iReason;
		std::string strUserID;
		std::string strDescription;
		std::string strExtraParam;
		seval_to_std_string(args[0], &strUserID);
		seval_to_int32(args[1], &iSource);
		seval_to_int32(args[2], &iReason);
		seval_to_std_string(args[3], &strDescription);
		seval_to_std_string(args[4], &strExtraParam);

		int iErrorCode = IM_Accusation(UTF8TOPlatString(strUserID).c_str(), (YIMChatType)iSource, iReason, UTF8TOPlatString(strDescription).c_str(), UTF8TOPlatString(strExtraParam).c_str());
		s.rval().setInt32(iErrorCode);
		return true;
	}
	SE_REPORT_ERROR( "wrong number of arguments: %d, was expecting %d", argc, 0);
	return false;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_Accusation)

static bool js_cocos2dx_extension_IM_GetMicrophoneStatus(se::State& s)
{
	IM_GetMicrophoneStatus();
	return true;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_GetMicrophoneStatus)

//获取禁言信息
static bool js_cocos2dx_extension_IM_GetForbiddenSpeakInfo(se::State& s) {
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 0) {
		

		int iErrorCode = IM_GetForbiddenSpeakInfo();
		s.rval().setInt32(iErrorCode);
		return true;
	}

	SE_REPORT_ERROR( "wrong number of arguments: %d, was expecting %d", argc, 1);
	return false;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_GetForbiddenSpeakInfo)

// 获取房间成员数量
static bool js_cocos2dx_extension_IM_GetRoomMemberCount(se::State& s){
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 1)
	{
		
		std::string roomID;
		seval_to_std_string(args[0], &roomID);

		int iErrorCode = IM_GetRoomMemberCount(UTF8TOPlatString(roomID).c_str());
		s.rval().setInt32(iErrorCode);
		return true;
	}
	SE_REPORT_ERROR( "wrong number of arguments: %d, was expecting %d", argc, 1);
	return false;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_GetRoomMemberCount)

// 屏蔽/解除屏蔽用户消息
static bool js_cocos2dx_extension_IM_BlockUser(se::State& s){
	const auto& args = s.args();
	size_t argc = args.size();
	if (argc == 2)
	{
		
		std::string userID;
		int block = 1;
		seval_to_std_string(args[0], &userID);
		seval_to_int32(args[1], &block);

		int iErrorCode = IM_BlockUser(UTF8TOPlatString(userID).c_str(), block);
		s.rval().setInt32(iErrorCode);
		return true;
	}
	SE_REPORT_ERROR( "wrong number of arguments: %d, was expecting %d", argc, 2);
	return false;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_BlockUser)

// 解除所有已屏蔽用户
static bool js_cocos2dx_extension_IM_UnBlockAllUser(se::State& s){
	const auto& args = s.args();
	size_t argc = args.size();
	
	int iErrorCode = IM_UnBlockAllUser();
	s.rval().setInt32(iErrorCode);
	return true;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_UnBlockAllUser)

// 获取房间成员数量
static bool js_cocos2dx_extension_IM_GetBlockUsers(se::State& s){
	const auto& args = s.args();
	size_t argc = args.size();
	int iErrorCode = IM_GetBlockUsers();
	s.rval().setInt32(iErrorCode);
	return true;
}
SE_BIND_FUNC(js_cocos2dx_extension_IM_GetBlockUsers)

bool register_jsb_youmeim(se::Object* obj) {
	auto cls = se::Class::create("YouMeIM", obj, nullptr, _SE(js_cocos2dx_extension_YouMeIM_constructor));

	cls->defineFunction("IM_Init", _SE(js_cocos2dx_extension_IM_Init));
	cls->defineFunction("IM_Uninit", _SE(js_cocos2dx_extension_IM_Uninit));
	//登陆等处
	cls->defineFunction("IM_Login", _SE(js_cocos2dx_extension_IM_Login));
	cls->defineFunction("IM_Logout", _SE(js_cocos2dx_extension_IM_Logout));
	//消息接口
	cls->defineFunction("IM_SendTextMessage", _SE(js_cocos2dx_extension_IM_SendTextMessage));
	cls->defineFunction("IM_SendCustomMessage", _SE(js_cocos2dx_extension_IM_SendCustomMessage));
	cls->defineFunction("IM_SendAudioMessage", _SE(js_cocos2dx_extension_IM_SendAudioMessage));
	cls->defineFunction("IM_SendOnlyAudioMessage", _SE(js_cocos2dx_extension_IM_SendOnlyAudioMessage));
	cls->defineFunction("IM_StopAudioMessage", _SE(js_cocos2dx_extension_IM_StopAudioMessage));
	cls->defineFunction("IM_CancleAudioMessage", _SE(js_cocos2dx_extension_IM_CancleAudioMessage));
	cls->defineFunction("IM_DownloadAudioFile", _SE(js_cocos2dx_extension_IM_DownloadAudioFile));
	cls->defineFunction("IM_DownloadFileByUrl", _SE(js_cocos2dx_extension_IM_DownloadFileByUrl));
	//聊天室
	cls->defineFunction("IM_JoinChatRoom", _SE(js_cocos2dx_extension_IM_JoinChatRoom));
	cls->defineFunction("IM_LeaveChatRoom", _SE(js_cocos2dx_extension_IM_LeaveChatRoom));
	cls->defineFunction("IM_SetAudioCacheDir", _SE(js_cocos2dx_extension_IM_SetAudioCacheDir));
	//获取SDK 版本号
	cls->defineFunction("IM_GetSDKVer", _SE(js_cocos2dx_extension_IM_GetSDKVer));
	//设置服务器区域
	cls->defineFunction("IM_SetServerZone", _SE(js_cocos2dx_extension_IM_SetServerZone));
	//关键词过滤
	cls->defineFunction("IM_GetFilterText", _SE(js_cocos2dx_extension_IM_GetFilterText));
	//是否自动播放
	cls->defineFunction("IM_SetAutoPlay", _SE(js_cocos2dx_extension_IM_SetAutoPlay));

	//仅录音上传接口，不需要发送
	cls->defineFunction("IM_StartAudioSpeech", _SE(js_cocos2dx_extension_IM_StartAudioSpeech));
	cls->defineFunction("IM_StopAudioSpeech", _SE(js_cocos2dx_extension_IM_StopAudioSpeech));
	cls->defineFunction("IM_QueryHistoryMessage", _SE(js_cocos2dx_extension_IM_QueryHistoryMessage));
	cls->defineFunction("IM_DeleteHistoryMessage", _SE(js_cocos2dx_extension_IM_DeleteHistoryMessage));
	cls->defineFunction("IM_DeleteHistoryMessageByID", _SE(js_cocos2dx_extension_IM_DeleteHistoryMessageByID));
	cls->defineFunction("IM_QueryRoomHistoryMessageFromServer", _SE(js_cocos2dx_extension_IM_QueryRoomHistoryMessageFromServer));
	cls->defineFunction("IM_ConvertAMRToWav", _SE(js_cocos2dx_extension_IM_ConvertAMRToWav));

	cls->defineFunction("IM_OnPause", _SE(js_cocos2dx_extension_IM_OnPause));
	cls->defineFunction("IM_OnResume", _SE(js_cocos2dx_extension_IM_OnResume));

	cls->defineFunction("IM_SendGift", _SE(js_cocos2dx_extension_IM_SendGift));
	cls->defineFunction("IM_MultiSendTextMessage", _SE(js_cocos2dx_extension_IM_MultiSendTextMessage));

	cls->defineFunction("IM_GetContact", _SE(js_cocos2dx_extension_IM_GetContact));
	cls->defineFunction("IM_GetUserInfo", _SE(js_cocos2dx_extension_IM_GetUserInfo));
	cls->defineFunction("IM_SetUserInfo", _SE(js_cocos2dx_extension_IM_SetUserInfo));

	cls->defineFunction("IM_SetReceiveMessageSwitch", _SE(js_cocos2dx_extension_IM_SetReceiveMessageSwitch));

	cls->defineFunction("IM_GetNewMessage", _SE(js_cocos2dx_extension_IM_GetNewMessage));

	cls->defineFunction("IM_QueryUserStatus", _SE(js_cocos2dx_extension_IM_QueryUserStatus));
	cls->defineFunction("IM_SetVolume", _SE(js_cocos2dx_extension_IM_SetVolume));
	cls->defineFunction("IM_StartPlayAudio", _SE(js_cocos2dx_extension_IM_StartPlayAudio));
	cls->defineFunction("IM_StopPlayAudio", _SE(js_cocos2dx_extension_IM_StopPlayAudio));
	cls->defineFunction("IM_IsPlaying", _SE(js_cocos2dx_extension_IM_IsPlaying));
	cls->defineFunction("IM_GetAudioCachePath", _SE(js_cocos2dx_extension_IM_GetAudioCachePath));
	cls->defineFunction("IM_ClearAudioCachePath", _SE(js_cocos2dx_extension_IM_ClearAudioCachePath));
	cls->defineFunction("IM_SetRoomHistoryMessageSwitch", _SE(js_cocos2dx_extension_IM_SetRoomHistoryMessageSwitch));

	cls->defineFunction("IM_TranslateText", _SE(js_cocos2dx_extension_IM_TranslateText));

	cls->defineFunction("IM_GetCurrentLocation", _SE(js_cocos2dx_extension_IM_GetCurrentLocation));
	cls->defineFunction("IM_GetNearbyObjects", _SE(js_cocos2dx_extension_IM_GetNearbyObjects));
	cls->defineFunction("IM_SetUpdateInterval", _SE(js_cocos2dx_extension_IM_SetUpdateInterval));

	cls->defineFunction("IM_SetSpeechRecognizeLanguage", _SE(js_cocos2dx_extension_IM_SetSpeechRecognizeLanguage));
	cls->defineFunction("IM_SetOnlyRecognizeSpeechText", _SE(js_cocos2dx_extension_IM_SetOnlyRecognizeSpeechText));
	cls->defineFunction("IM_GetMicrophoneStatus", _SE(js_cocos2dx_extension_IM_GetMicrophoneStatus));
	cls->defineFunction("IM_Accusation", _SE(js_cocos2dx_extension_IM_Accusation));
	cls->defineFunction("IM_QueryNotice", _SE(js_cocos2dx_extension_IM_QueryNotice));

	cls->defineFunction("IM_GetForbiddenSpeakInfo", _SE(js_cocos2dx_extension_IM_GetForbiddenSpeakInfo));

	cls->defineFunction("IM_GetRoomMemberCount", _SE(js_cocos2dx_extension_IM_GetRoomMemberCount));
	cls->defineFunction("IM_BlockUser", _SE(js_cocos2dx_extension_IM_BlockUser));
	cls->defineFunction("UnBlockAllUser", _SE(js_cocos2dx_extension_IM_UnBlockAllUser));
	cls->defineFunction("IM_GetBlockUsers", _SE(js_cocos2dx_extension_IM_GetBlockUsers));

	cls->defineFinalizeFunction(_SE(js_cocos2dx_YouMeIM_finalize));
	cls->install();
	//JSBClassType::registerClass<YouMeIM>(cls);

	__jsb_YouMeIM_proto = cls->getProto();
	__jsb_YouMeIM_class = cls;

	se::ScriptEngine::getInstance()->clearException();
	return true;
}
