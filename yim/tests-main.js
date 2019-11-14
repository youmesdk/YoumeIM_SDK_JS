/****************************************************************************
 Copyright (c) 2008-2010 Ricardo Quesada
 Copyright (c) 2011-2012 cocos2d-x.org
 Copyright (c) 2013-2014 Chukong Technologies Inc.

 http://www.cocos2d-x.org


 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

// globals
var director = null;
var winSize = null;

var PLATFORM_JSB = 1 << 0;
var PLATFORM_HTML5 = 1 << 1;
var PLATFORM_HTML5_WEBGL = 1 << 2;
var PLATFROM_ANDROID = 1 << 3;
var PLATFROM_IOS = 1 << 4;
var PLATFORM_MAC = 1 << 5;
var PLATFORM_JSB_AND_WEBGL =  PLATFORM_JSB | PLATFORM_HTML5_WEBGL;
var PLATFORM_ALL = PLATFORM_JSB | PLATFORM_HTML5 | PLATFORM_HTML5_WEBGL | PLATFROM_ANDROID | PLATFROM_IOS;
var PLATFROM_APPLE = PLATFROM_IOS | PLATFORM_MAC;

// automation vars
var autoTestEnabled = autoTestEnabled || false;
var autoTestCurrentTestName = autoTestCurrentTestName || "N/A";


var youmeim = new YouMeIM
//登陆登出回掉
youmeim.OnLogin = function (errorcode,youmeID)
{
	cc.log("errorcode:" + errorcode + " youmeID:" + youmeID)
}

youmeim.OnLogout = function ()
{
	cc.log("OnLogout")
}

youmeim.OnKickOff = function ()
{
	cc.log("OnKickOff")
}

youmeim.IM_SetAutoPlay(1)
//下载语音文件
youmeim.OnDownload = function ( errorcode, strSavepath, bodytype,chattype, serial,recvid,senderid,content,params,duration,createtime ){
    cc.log("下载文件回调");
    if( errorcode === 0 ){
        cc.log("下载文件 %s 成功 ", strSavepath );

        cc.log("body:%d,chat:%d,serial:%d", bodytype, chattype, serial )
        cc.log("rec:%s, send:%s", recvid, senderid )
        cc.log("cont:%s", content )
        cc.log("params:%s", params )
        cc.log("duration:%d", duration)
        cc.log("createtime:%d", createtime )

        //通知场景
        event = new cc.EventCustom("DownloadVoice");
        msg = {
            msgID : serial,
            voicePath : strSavepath
        };
        event.setUserData( msg );
        cc.eventManager.dispatchEvent( event );
    }
    else{
        cc.log("下载文件 %s 失败, error = %d  ", strSavepath, errorcode);
    }
};

youmeim.OnDownloadByUrl = function ( errorcode , fromUrl, savePath ){
    cc.log("下载文件回调:");
    cc.log("from:%s", fromUrl )
    cc.log("savePath:%s", savePath )
    if( errorcode === 0 ){
        cc.log("下载文件成功")
    }
    else{
        cc.log("下载文件 %s 失败, error = %d  ", strSavepath, errorcode);
    }
};

youmeim.OnQueryHistoryMsg = function(targetID,remainCount,msgLists)
{
	for(var i=0;i<msgLists.length;i++)
	{
		cc.log("OnQueryHistoryMsg： chattype：" + msgLists[i].ChatType + "Duration:" + msgLists[i].Duration + "|" + msgLists[i].MessageType + "|" +  msgLists[i].Param + "|" +  msgLists[i].ReceiveID + "|" +  msgLists[i].SenderID+  msgLists[i].Serial+  msgLists[i].Text + msgLists[i].LocalPath)
	}
	
}
youmeim.OnGetContactList = function(errorcode, contacts)
{
	//查询最近联系人列表的回调
    cc.log("查询最近联系人列表信息 / errorcode: " + errorcode + " count: " + contacts.length);
    
    for (var i = 0; i < contacts.length; i++)
    {
        var obj = contacts[i];

        cc.log( " contactID: " + obj.ContactID );
        cc.log( " messageType: " + obj.MessageType );
        cc.log( " messageContent: " + obj.MessageContent );
        cc.log( " creatTime: " + obj.CreateTime );
    }	
}


youmeim.OnAudioSpeech = function(serial,errorcode,strDownloadURL,iDuration,iFileSize,strLocalPath,strText)
{
	cc.log("OnAudioSpeech" + serial + " errorcode:" + errorcode + " url:" + strDownloadURL + "Duration:" + iDuration + "FileSize:" + iFileSize + "LocalPath:" + strLocalPath + "text:" + strText)
}

//消息回掉. 普通消息发送回掉接口
youmeim.OnSendMessageStatus = function(serial, errorcode, sendTime, isForbidRoom, reasonType, endTime)
{
	cc.log("OnSendMessageStatus" + serial + " errorcode:" + errorcode)
}

youmeim.OnStartSendAudioMessage = function( serial, errorcode, text, path, duration ){
    cc.log("OnStartSendAudioMessage："+ serial+" ，"+ errorcode +" ，" + text +" ，"+path + " ，" + duration);
}

youmeim.OnSendAudioMessageStatus = function(serial,errorcode,content,localpath,duration, sendTime, isForbidRoom, reasonType, endTime)
{
	cc.log("OnSendAudioMessageStatus" + serial + " errorcode:" + errorcode + " content:" + content + " localpath:" + localpath +" duration:" + duration)
}

youmeim.OnRecvMessage = function(bodytype,chattype, serial,recvid,senderid,content,params,duration,createtime)
{
	cc.log("OnRecvMessage" + bodytype + " chattype:" + chattype + " serial:" + serial + " recvid:" + recvid + " senderid:" + senderid + " content:" + content + " param:" + params )
	//youmeim.IM_DownloadAudioFile(serial,"C:\\testjs\\MyJSGame\\frameworks\\cocos2d-x\\build\\abc.wav")
}

//群组回掉
youmeim.OnJoinChatroom = function(errorcode,groupid)
{
	cc.log("OnJoinChatroom:" + groupid + " errorcode:" + errorcode)
}

youmeim.OnRecvMessageNotify = function( chattype, targetID )
{
	
	cc.log("OnRecvMessageNotify:" + chattype + " , " + targetID )
	
}

youmeim.OnLeaveAllChatRooms = function (errorcode)
{
    cc.log("OnLeaveAllChatRooms errorcode:" + errorcode);
}

youmeim.OnGetUserInfo = function( errorcode, userid, jsonUserInfo ){
    cc.log("获得信息回调:%d userid:%s", errorcode, userid);
    cc.log("json:" + jsonUserInfo );
    var obj = JSON.parse( jsonUserInfo );
    cc.log( obj.nickname );
    cc.log( obj.server_area );
    cc.log( obj.location );
    cc.log( obj.level );
    cc.log( obj.vip_level );
    cc.log( obj.extra );
}

youmeim.OnQueryUserStatus = function (  errorcode, userid, state  )
{
    cc.log( "查询用户登录状态,"+ errorcode + ",userid:" + userid + "," + state );
    //通知有新消息，只有在手动接受消息的模式下才有用
};

youmeim.OnPlayCompletion = function (  errorcode, path  ){
    cc.log( "播放结束:"+ errorcode + ",Path:" + path  );
};

// 文本翻译回调
youmeim.OnTranslateTextComplete = function(errorcode, requestID, text, srcLangCode, destLangCode){
    cc.log( "OnTranslateTextComplete:"+ errorcode + ",requestID:" + requestID + ",text:" + text + ",srcLangCode" + srcLangCode + ",destLangCode" + destLangCode );
};

// 获取当前地理位置回调
youmeim.OnUpdateLocation = function(districtCode, country, pronvice, city, districtCounty, street, longitude, latitude){
    cc.log( "OnUpdateLocation districtCode:" + districtCode + ",country:" + country + ",pronvice" + pronvice + ",city:" + city + ",longitude:" + longitude + ",latitude:" + latitude);
};

// 查找附近的人回调
youmeim.OnGetNearbyObjects = function(errorcode, startDistance, endDistance, neighbourList){
    cc.log( "OnGetNearbyObjects errorcode:"+ errorcode + ",startDistance:" + startDistance + ",endDistance" + endDistance);
	for(var i=0;i<neighbourList.length;i++)
	{
		cc.log("Distance:" + neighbourList[i].Distance + " Longitude:" + neighbourList[i].Longitude +" Latitude:" + neighbourList[i].Latitude +" UserID:" + neighbourList[i].UserID +" Country:" + neighbourList[i].Country +" Province:" + neighbourList[i].Province +" City:" + neighbourList[i].City +" DistrictCounty:" + neighbourList[i].DistrictCounty)
	}
};

// 用户进入房间回调
youmeim.OnUserJoinChatRoom = function(channelID, userID){
    cc.log( "OnUserJoinChatRoom channelID:"+ channelID + " userID:" + userID);
};

// 用户退出房间回调
youmeim.OnUserLeaveChatRoom = function(channelID, userID){
    cc.log( "OnUserLeaveChatRoom channelID:"+ channelID + " userID:" + userID);
};

// 获取麦克风状态回调
youmeim.OnGetMicrophoneStatus = function(status){
    cc.log( "OnGetMicrophoneStatus status:"+ status );
};

// 举报处理通知
youmeim.OnAccusationResultNotify = function(result, userID, accusationTime){
    cc.log("OnAccusationResultNotify result:"+ result + " userID:" + userID + " accusationTime:" + accusationTime);
};

// 接收公告
youmeim.OnRecvNotice = function(noticeID, channelID, noticeType, content, linkText, linkAddress, beginTime, endTime){
    cc.log("OnRecvNotice noticeID:"+ noticeID + " channelID:" + channelID + " noticeType:" + noticeType + " content:" + content + " beginTime:" + beginTime + " endTime:" + endTime);
};

// 撤销公告
youmeim.OnCancelNotice = function(noticeID, channelID){
     cc.log("OnCancelNotice noticeID:"+ noticeID + " channelID:" + channelID);
};

youmeim.OnGetForbiddenSpeakInfo = function( errorcode , forbiddenInfos )
{   
    cc.log("禁言查询结果:" + errorcode + ",len:" + forbiddenInfos.length );
    
    for( var i=0;i<forbiddenInfos.length;i++ ){
        var obj = forbiddenInfos[i];

        cc.log("禁言：" + obj.channelID + "," + obj.isForbidRoom + "," + obj.reasonType +"," + obj.endTime  );

    }
}

//获取房间成员数量回调
youmeim.OnGetRoomMemberCount = function(errorcode, roomID, count){
	cc.log("OnGetRoomMemberCount errorcode:" + errorcode + " roomID:" + roomID + " count:" + count);
}

//屏蔽/解除屏蔽用户消息回调
youmeim.OnBlockUser = function(errorcode, userID, block){
	cc.log("OnBlockUser errorcode:" + errorcode + " userID:" + userID + " block:" + block);
}

//解除所有已屏蔽用户回调
youmeim.OnUnBlockAllUser = function(errorcode){
	cc.log("OnUnBlockAllUser errorcode:" + errorcode);
}

//获取被屏蔽消息用户回调
youmeim.OnGetBlockUsers = function(errorcode, userList){
	cc.log("OnGetBlockUsers errorcode:" + errorcode);
	for(var i=0;i<userList.length;i++){
		cc.log("OnGetBlockUsers userID:" + userList[i]);
	}
}

//获取识别语音的文字
youmeim.OnGetRecognizeSpeechText = function(serial, errorcode, text)
{
    cc.log("OnGetRecognizeSpeechText serial:" + serial + " errorcode:" + errorcode + " content:" + text);    
}

// 开始重连回调
youmeim.OnStartReconnect = function(){
    cc.log("OnStartReconnect");
}

// 重连结果回调
youmeim.OnRecvReconnectResult = function(result){
    cc.log("OnRecvReconnectResult result:"+ result);
}

// 带文字识别的录音音量回调
youmeim.OnRecordVolume = function(volume){
    cc.log("OnRecordVolume volume:"+volume);
}

//获取与指定用户距离回调
youmeim.OnGetDistance = function(errorcode, userID, distance)
{
    cc.log("OnGetDistance errorcode:" + errorcode + " userID:" + userID + " distance:" + distance);
}

//从服务器查询房间历史消息回调
youmeim.OnQueryRoomHistoryMessageFromServer = function(errorcode, roomID, remain, messageList)
{
    cc.log("OnQueryRoomHistoryMessageFromServer errorcode:" + errorcode + " roomID:" + roomID + " remain:" + remain + " size:" + messageList.length);
	
	for(var i=0;i<messageList.length;i++)
	{
		cc.log("messageID:" + messageList[i].MessageID + " chatType:" + messageList[i].ChatType + " receiveID:" + messageList[i].ReceiveID + " senderID:" + messageList[i].SenderID + " content:" + messageList[i].Content + " extraParam:" + messageList[i].ExtraParam)
		
		if(messageList[i].MessageType == 1)		//文本
		{
			
		}
		else if(messageList[i].MessageType == 2)	//自定义
		{
			
		}
		else if(messageList[i].MessageType == 5)	//语音
		{
			cc.log("AudioDuration:" + messageList[i].AudioDuration);
		}
		else if(messageList[i].MessageType == 7)	//文件
		{
			cc.log("FileType:" + messageList[i].FileType + " FileSize:" + messageList[i].FileSize + " FileName:" + messageList[i].FileName + " FileExtension:" + messageList[i].FileExtension);
		}
		else if(messageList[i].MessageType == 8)	//礼物
		{
			cc.log("GiftID:" + messageList[i].GiftID + " GiftCount:" + messageList[i].GiftCount + " Anchor:" + messageList[i].Anchor);
		}
	}
}
//查询用户基本资料回调
youmeim.OnQueryUserInfo = function (errorcode,userID,photoUrl,onlineState,beAddPermission,foundPermission,nickname,sex,signature,country,province,city,extraInfo )
{
    cc.log("OnQueryUserInfo, errorcode: %d,userID：%s,photoURL：%s,onlineState：%s，beAddPermission：%d,foundPermission：%d,nickName：%s，sex：%d,signature：%s,country：%s,province：%s,city：%s", errorcode,userID,photoUrl,onlineState,beAddPermission,foundPermission,nickname,sex,signature,country,province,city);
}

//设置用户基本资料回调
youmeim.OnSetUserInfo = function (errorcode)
{
    cc.log("OnSetUserInfo, errorcode: %d", errorcode);
}
//切换用户在线状态回调
youmeim.OnSwitchUserOnlineState = function (errorcode)
{
    cc.log("OnSwitchUserOnlineState, errorcode: %d", errorcode);
}
//设置用户头像回调
youmeim.OnSetPhotoUrl = function (errorcode,photoUrl)
{
    cc.log("OnSetPhotoUrl, errorcode: %d，photoURL：%s", errorcode,photoUrl);
}
//用户资料变更通知
youmeim.OnUserInfoChangeNotify = function (userID)
{
    cc.log("OnUserInfoChangeNotify, userID: %s", userID);
}
//查找用户回调
youmeim.OnFindUser = function (errorcode, userList)
{
    cc.log("OnFindUser， errorcode: %d", errorcode);
    var users = [];
    for (var i = 0; i < userList.length; i++)
    {
        var obj = userList[i];
        
        cc.log( " userID: " + obj.UserID );
        cc.log( " nickName: " + obj.NickName );
        cc.log( " status: " + obj.Status );
        users.push(obj.UserID);
    }
    var ymErrorcode = youmeim.IM_RequestAddFriend(users,"please pass.");
    if( 0  === ymErrorcode)
    {
        cc.log("RequestAddFriend success.\n");
    }
    else
    {
        cc.log("RequestAddFriend fail,errorcode：%d\n",  ymErrorcode);
    }
}
//请求添加好友回调
youmeim.OnRequestAddFriend = function (errorcode,userID)
{
    cc.log("OnRequestAddFriend, errorcode:%d,userID: %s", errorcode,userID);
}
//被请求添加为好友通知
youmeim.OnBeRequestAddFriendNotify = function (userID,comments)
{
    cc.log("OnBeRequestAddFriendNotify， userID: %s,comments:%s", userID,comments);
    var ymErrorcode = youmeim.IM_DealBeRequestAddFriend(userID,0);
    if( 0  === ymErrorcode)
    {
        cc.log("DealBeRequestAddFriend success.\n");
    }
    else
    {
        cc.log("DealBeRequestAddFriend fail,errorcode:%d\n",  ymErrorcode);
    }
}
//被添加为好友通知
youmeim.OnBeAddFriendNotify = function (userID,comments)
{
    cc.log("OnBeAddFriendNotify， userID: %s,comments:%s", userID,comments);
    var ymErrorcode = youmeim.IM_QueryFriends(0,0,5);
    if( 0  === ymErrorcode)
    {
        cc.log("QueryFriends success.\n");
    }
    else
    {
        cc.log("QueryFriends fail,errorcode:%d\n",  ymErrorcode);
    }
}
//处理被添加为好友回调
youmeim.OnDealBeRequestAddFriend = function (errorcode,userID,comments,dealResult)
{
    cc.log("OnDealBeRequestAddFriend,errorcode:%d,userID: %s,comments:%s,dealResult:%d", errorcode,userID,comments,dealResult);
    if(0 === dealResult)
    {
        var ymErrorcode = youmeim.IM_QueryFriends(0,0,5);
        if( 0  === ymErrorcode)
        {
            cc.log("QueryFriends success.\n");
        }
        else
        {
            cc.log("QueryFriends fail,errorcode:%d\n",  ymErrorcode);
        }
    }
}
//请求添加好友结果通知
youmeim.OnRequestAddFriendResultNotify = function (userID,comments,dealResult)
{
    cc.log("OnRequestAddFriendResultNotify, userID: %s,comments:%s,dealResult:%d", userID,comments,dealResult);
    if (0 === dealResult)
    {
        var ymErrorcode = youmeim.IM_QueryFriends(0,0,5);
        if( 0  === ymErrorcode)
        {
            cc.log("QueryFriends success.\n");
        }
        else
        {
            cc.log("QueryFriends fail,errorcode：%d\n",  ymErrorcode);
        }
    }
}
//删除好友回调
youmeim.OnDeleteFriend = function (errorcode,userID)
{
    cc.log("OnDeleteFriend，errorcode:%d, userID: %s", errorcode,userID);
    var ymErrorcode = youmeim.IM_QueryFriends(0,0,5);
    if( 0  === ymErrorcode)
    {
        cc.log("QueryFriends success.\n");
    }
    else
    {
        cc.log("QueryFriends fail, errorcode:%d\n",  ymErrorcode);
    }
}
//被好友删除通知
youmeim.OnBeDeleteFriendNotify = function (userID)
{
    cc.log("OnBeDeleteFriendNotify, userID: %s", userID);
    var ymErrorcode = youmeim.IM_QueryFriends(0,0,5);
    if( 0  === ymErrorcode)
    {
        cc.log("QueryFriends success.\n");
    }
    else
    {
        cc.log("QueryFriends fail,errorcode：%d\n",  ymErrorcode);
    }
}
//拉黑好友回调
youmeim.OnBlackFriend = function (errorcode,type,userID)
{
    cc.log("OnBlackFriend，errorcode:%d, type:%d, userID: %s", errorcode,type,userID);
    var ymErrorcode = youmeim.IM_QueryFriends(1,0,5);
    if( 0  === ymErrorcode)
    {
        cc.log("Query my blacked friends success.\n");
    }
    else
    {
        cc.log("Query my blacked friends fail,errorcode:%d\n",  ymErrorcode);
    }
}
//查询我的好友回调
youmeim.OnQueryFriends = function (errorcode,type,startIndex,friendList)
{
    cc.log("OnQueryFriends,errorcode:%d,type: %d,startIndex:%d", errorcode,type,startIndex);
    for (var i = 0; i < friendList.length; i++)
    {
        var obj = friendList[i];
        
        cc.log( " userID: " + obj.UserID );
        cc.log( " nickName: " + obj.NickName );
        cc.log( " status: " + obj.Status );
    }
}
//查询好友请求列表回调
youmeim.OnQueryFriendRequestList = function (errorcode,startIndex,requestList)
{
    cc.log("OnQueryFriendRequestList，errorcode:%d,startIndex:%d", errorcode,startIndex);
    for (var i = 0; i < requestList.length; i++)
    {
        var obj = requestList[i];
        
        cc.log( " aserID: " + obj.AskerID );
        cc.log( " askerNickName: " + obj.AskerNickName );
        cc.log( " inviteeID: " + obj.InviteeID );
        cc.log( " inviteeNickName: " + obj.InviteeNickName );
        cc.log( " validateInfo: " + obj.ValidateInfo );
        cc.log( " addStatus: " + obj.Status );
        cc.log( " createTime: " + obj.CreateTime );
    }
}


var errorcode = youmeim.IM_Init("YOUMEBC2B3171A7A165DC10918A7B50A4B939F2A187D0","r1+ih9rvMEDD3jUoU+nj8C7VljQr7Tuk4TtcByIdyAqjdl5lhlESU0D+SoRZ30sopoaOBg9EsiIMdc8R16WpJPNwLYx2WDT5hI/HsLl1NJjQfa9ZPuz7c/xVb8GHJlMf/wtmuog3bHCpuninqsm3DRWiZZugBTEj2ryrhK7oZncBAAE=");
cc.log("imiterrorcode " + errorcode)
errorcode = youmeim.IM_Login("joexie","123456","")
var targets = ["123"]
youmeim.IM_SetReceiveMessageSwitch(targets, 0);
cc.log("IM_Login: " + errorcode)
var TestScene = cc.Scene.extend({
    ctor:function (bPortrait) {
        this._super();
        this.init();

        var label = new cc.LabelTTF("Main Menu", "Arial", 20);
        var menuItem = new cc.MenuItemLabel(label, this.onMainMenuCallback, this);

        var menu = new cc.Menu(menuItem);
        menu.x = 0;
        menu.y = 0;
        menuItem.x = winSize.width - 50;
        menuItem.y = 25;

        if(!window.sideIndexBar){
            this.addChild(menu, 1);
        }
    },
    onMainMenuCallback:function () {
        if (director.isPaused()) {
            director.resume();
        } 
        var scene = new cc.Scene();
        var layer = new TestController();
        scene.addChild(layer);
        var transition = new cc.TransitionProgressRadialCCW(0.5,scene);
        director.runScene(transition);
    },

    runThisTest:function () {
        // override me
    }

});

//Controller stuff
var LINE_SPACE = 40;
var curPos = cc.p(0,0);
var startAudio = false
var TestController = cc.LayerGradient.extend({
    _itemMenu:null,
    _beginPos:0,
    isMouseDown:false,

    ctor:function() {
        this._super(cc.color(0,0,0,255), cc.color(0x46,0x82,0xB4,255));

        // globals
        director = cc.director;
        winSize = director.getWinSize();

        // add close menu
        var closeItem = new cc.MenuItemImage(s_pathClose, s_pathClose, this.onCloseCallback, this);
        closeItem.x = winSize.width - 30;
        closeItem.y = winSize.height - 30;

        var subItem1 = new cc.MenuItemFont("Automated Test: Off");
        subItem1.fontSize = 18;
        var subItem2 = new cc.MenuItemFont("Automated Test: On");
        subItem2.fontSize = 18;

        var toggleAutoTestItem = new cc.MenuItemToggle(subItem1, subItem2);
        toggleAutoTestItem.setCallback(this.onToggleAutoTest, this);
        toggleAutoTestItem.x = winSize.width - toggleAutoTestItem.width / 2 - 10;
        toggleAutoTestItem.y = 20;
        toggleAutoTestItem.setVisible(false);
        if( autoTestEnabled )
            toggleAutoTestItem.setSelectedIndex(1);


        var menu = new cc.Menu(closeItem, toggleAutoTestItem);//pmenu is just a holder for the close button
        menu.x = 0;
        menu.y = 0;

        // sort the test title
        testNames.sort(function(first, second){
            if (first.title > second.title)
            {
                return 1;
            }
            return -1;
        });

        // add menu items for tests
        this._itemMenu = new cc.Menu();//item menu is where all the label goes, and the one gets scrolled

        for (var i = 0, len = testNames.length; i < len; i++) {
            var label = new cc.LabelTTF(testNames[i].title, "Arial", 24);
            var menuItem = new cc.MenuItemLabel(label, this.onMenuCallback, this);
            this._itemMenu.addChild(menuItem, i + 10000);
            menuItem.x = winSize.width / 2;
            menuItem.y = (winSize.height - (i + 1) * LINE_SPACE);

            // enable disable
            if ( !cc.sys.isNative) {
                if( cc._renderType !== cc.game.RENDER_TYPE_CANVAS ){
                    menuItem.enabled = (testNames[i].platforms & PLATFORM_HTML5) | (testNames[i].platforms & PLATFORM_HTML5_WEBGL);
                }else{
                    menuItem.setEnabled( testNames[i].platforms & PLATFORM_HTML5 );
                }
            } else {
                if (cc.sys.os == cc.sys.OS_ANDROID) {
                    menuItem.setEnabled( testNames[i].platforms & ( PLATFORM_JSB | PLATFROM_ANDROID ) );
                } else if (cc.sys.os == cc.sys.OS_IOS) {
                    menuItem.setEnabled( testNames[i].platforms & ( PLATFORM_JSB | PLATFROM_IOS) );
                } else if (cc.sys.os == cc.sys.OS_OSX) {
                    menuItem.setEnabled( testNames[i].platforms & ( PLATFORM_JSB | PLATFORM_MAC) );
                } else {
                    menuItem.setEnabled( testNames[i].platforms & PLATFORM_JSB );
                }
            }
        }

        this._itemMenu.width = winSize.width;
        this._itemMenu.height = (testNames.length + 1) * LINE_SPACE;
        this._itemMenu.x = curPos.x;
        this._itemMenu.y = curPos.y;
        this.addChild(this._itemMenu);
        this.addChild(menu, 1);

        // 'browser' can use touches or mouse.
        // The benefit of using 'touches' in a browser, is that it works both with mouse events or touches events
        if ('touches' in cc.sys.capabilities) {
            cc.eventManager.addListener({
                event: cc.EventListener.TOUCH_ALL_AT_ONCE,
                onTouchesMoved: function (touches, event) {
                    var target = event.getCurrentTarget();
                    var delta = touches[0].getDelta();
                    target.moveMenu(delta);
                    return true;
                }
            }, this);
        }
        else if ('mouse' in cc.sys.capabilities) {
            cc.eventManager.addListener({
                event: cc.EventListener.MOUSE,
                onMouseMove: function (event) {
                    if(event.getButton() == cc.EventMouse.BUTTON_LEFT)
                        event.getCurrentTarget().moveMenu(event.getDelta());
                },
                onMouseScroll: function (event) {
                    var delta = cc.sys.isNative ? event.getScrollY() * 6 : -event.getScrollY();
                    event.getCurrentTarget().moveMenu({y : delta});
                    return true;
                }
            }, this);
        }
    },
    onEnter:function(){
        this._super();
	    this._itemMenu.y = TestController.YOffset;
    },
    onMenuCallback:function (sender) {
        TestController.YOffset = this._itemMenu.y;
        var idx = sender.getLocalZOrder() - 10000;
        // get the userdata, it's the index of the menu item clicked
        // create the test scene and run it

        autoTestCurrentTestName = testNames[idx].title;

        var testCase = testNames[idx];
        var res = testCase.resource || [];
        cc.LoaderScene.preload(res, function () {
            var scene = testCase.testScene();
            if (scene) {
                scene.runThisTest();
            }
        }, this);
    },
    onCloseCallback:function () {
		/*if (startAudio)
		{
			youmeim.IM_StopAudioSpeech("123")
			startAudio = false;
		}
		else
		{
			youmeim.IM_StartAudioSpeech(1)
			startAudio=true;
		
		}*/	
		
		//youmeim.IM_MultiSendTextMessage(["123","3456","43sdf"],"extparam")
		//youmeim.IM_GetContact()
		//youmeim.IM_JoinChatRoom("100");
        var targets = ["123"]
		youmeim.IM_SetReceiveMessageSwitch( targets, 1);
    },
    onToggleAutoTest:function() {
        autoTestEnabled = !autoTestEnabled;
    },

    moveMenu:function(delta) {
        var newY = this._itemMenu.y + delta.y;
        if (newY < 0 )
            newY = 0;

        if( newY > ((testNames.length + 1) * LINE_SPACE - winSize.height))
            newY = ((testNames.length + 1) * LINE_SPACE - winSize.height);

        this._itemMenu.y = newY;
    }
});
TestController.YOffset = 0;
var testNames = [
   
    {
        title:"BillBoard Test",
        platforms: PLATFORM_JSB,
        linksrc:"src/BillBoardTest/BillBoardTest.js",
        testScene:function () {
            return new BillBoardTestScene();
        }
    },
    {
        title:"Box2D Test",
        resource:g_box2d,
        platforms: PLATFORM_HTML5,
        linksrc:"src/Box2dTest/Box2dTest.js",
        testScene:function () {
            return new Box2DTestScene();
        }
    },
    {
        title:"Camera3D Test",
        platforms: PLATFORM_JSB,
        linksrc:"src/Camera3DTest/Camera3DTest.js",
        testScene:function () {
            return new Camera3DTestScene();
        }
    },
    {
        title:"Chipmunk Test",
        platforms: PLATFORM_ALL,
        linksrc:"src/ChipmunkTest/ChipmunkTest.js",
        testScene:function () {
            return new ChipmunkTestScene();
        }
    },
    {
        title:"ClippingNode Test",
        platforms: PLATFORM_ALL,
        linksrc:"src/ClippingNodeTest/ClippingNodeTest.js",
        testScene:function () {
            return new ClippingNodeTestScene();
        }
    },
    {
        title:"CocosDenshion Test",
        resource:g_cocosdeshion,
        platforms: PLATFORM_ALL,
        linksrc:"src/CocosDenshionTest/CocosDenshionTest.js",
        testScene:function () {
            return new CocosDenshionTestScene();
        }
    },
    {
        title:"Component Test",
        platforms: PLATFORM_JSB,
        linksrc:"src/ComponentTest/ComponentTest.js",
        testScene:function () {
            return new ComponentTestScene();
        }
    },
    {
        title:"CurrentLanguage Test",
        platforms: PLATFORM_ALL,
        linksrc:"src/CurrentLanguageTest/CurrentLanguageTest.js",
        testScene:function () {
            return new CurrentLanguageTestScene();
        }
    },
    //"CurlTest",
    {
        title:"DrawPrimitives Test",
        platforms: PLATFORM_ALL,
        linksrc:"src/DrawPrimitivesTest/DrawPrimitivesTest.js",
        testScene:function () {
            return new DrawPrimitivesTestScene();
        }
    },
    {
        title:"EaseActions Test",
        platforms: PLATFORM_ALL,
        linksrc:"src/EaseActionsTest/EaseActionsTest.js",
        testScene:function () {
            return new EaseActionsTestScene();
        }
    },
    {
        title:"Event Manager Test",
        resource:g_eventDispatcher,
        platforms: PLATFORM_ALL,
        linksrc:"src/NewEventManagerTest/NewEventManagerTest.js",
        testScene:function () {
            return new EventDispatcherTestScene();
        }
    },
    {
        title:"Event Test",
        platforms: PLATFORM_ALL,
        linksrc:"src/EventTest/EventTest.js",
        testScene:function () {
            return new EventTestScene();
        }
    },
    {
        title:"Extensions Test",
        resource:g_extensions,
        platforms: PLATFORM_ALL,
        linksrc:"",
        testScene:function () {
            return new ExtensionsTestScene();
        }
    },
    {
        title:"Effects Test",
        platforms: PLATFORM_JSB_AND_WEBGL,
        linksrc:"src/EffectsTest/EffectsTest.js",
        testScene:function () {
            return new EffectsTestScene();
        }
    },
    {
        title:"Effects Advanced Test",
        platforms: PLATFORM_JSB_AND_WEBGL,
        linksrc:"src/EffectsAdvancedTest/EffectsAdvancedTest.js",
        testScene:function () {
            return new EffectAdvanceScene();
        }
    },
    {
        title:"Native Test",
        platforms: PLATFORM_JSB,
        linksrc:"src/NativeTest/NativeTest.js",
        testScene:function () {
            return new NativeTestScene();
        }
    },
    //{
    //    title:"Facebook SDK Test",
    //    platforms: PLATFROM_ANDROID | PLATFROM_IOS | PLATFORM_HTML5,
    //    linksrc:"src/FacebookTest/FacebookTestsManager.js",
    //    testScene:function () {
    //        return new FacebookTestScene();
    //    }
    //},
    {
        title:"Font Test",
        resource:g_fonts,
        platforms: PLATFORM_ALL,
        linksrc:"src/FontTest/FontTest.js",
        testScene:function () {
            return new FontTestScene();
        }
    },
    {
        title:"UI Test",
        resource:g_ui,
        platforms: PLATFORM_ALL,
        linksrc:"",
        testScene:function () {
            return new GUITestScene();
        }
    },
    //"HiResTest",
    {
        title:"Interval Test",
        platforms: PLATFORM_ALL,
        linksrc:"src/IntervalTest/IntervalTest.js",
        testScene:function () {
            return new IntervalTestScene();
        }
    },
    {
        title:"Label Test",
        resource:g_label,
        platforms: PLATFORM_ALL,
        linksrc:"src/LabelTest/LabelTest.js",
        testScene:function () {
            return new LabelTestScene();
        }
    },
    {
        title:"Layer Test",
        platforms: PLATFORM_ALL,
        linksrc:"src/LayerTest/LayerTest.js",
        testScene:function () {
            return new LayerTestScene();
        }
    },
    {
        title:"Light Test",
        platforms: PLATFORM_JSB,
        linksrc:"src/LightTest/LightTest.js",
        testScene:function () {
            return new LightTestScene();
        }
    },
    {
        title:"Loader Test",
        platforms: PLATFORM_ALL,
        linksrc:"src/LoaderTest/LoaderTest.js",
        testScene:function () {
            return new LoaderTestScene();
        }
    },
    {
        title:"MaterialSystem Test",
        platforms: PLATFORM_JSB,
        linksrc:"src/MaterialSystemTest/MaterialSystemTest.js",
        testScene:function () {
            return new MaterialSystemTestScene();
        }
    },
    {
        title:"Memory Model Test",
        resource:g_menu,
        platforms: PLATFORM_JSB,
        linksrc:"src/MemoryModelTest/MemoryModelTest.js",
        testScene:function () {
            return new MemoryModelTestScene();
        }
    },
    {
        title:"Menu Test",
        resource:g_menu,
        platforms: PLATFORM_ALL,
        linksrc:"src/MenuTest/MenuTest.js",
        testScene:function () {
            return new MenuTestScene();
        }
    },
    {
        title:"MotionStreak Test",
        platforms: PLATFORM_JSB_AND_WEBGL,
        linksrc:"src/MotionStreakTest/MotionStreakTest.js",
        testScene:function () {
            return new MotionStreakTestScene();
        }
    },
    {
        title:"Node Test",
        platforms: PLATFORM_ALL,
        linksrc:"src/CocosNodeTest/CocosNodeTest.js",
        testScene:function () {
            return new NodeTestScene();
        }
    },
    {
        title:"OpenGL Test",
        resource:g_opengl_resources,
        platforms: PLATFORM_JSB_AND_WEBGL,
        linksrc:"src/OpenGLTest/OpenGLTest.js",
        testScene:function () {
            return new OpenGLTestScene();
        }
    },
    {
        title:"Parallax Test",
        resource:g_parallax,
        platforms: PLATFORM_ALL,
        linksrc:"src/ParallaxTest/ParallaxTest.js",
        testScene:function () {
            return new ParallaxTestScene();
        }
    },
    {
        title:"Particle3D Test",
        platforms: PLATFORM_JSB,
        testScene:function () {
            return new Particle3DTestScene();
        }
    },
    {
        title:"Particle Test",
        platforms: PLATFORM_ALL,
        linksrc:"",
        resource:g_particle,
        testScene:function () {
            return new ParticleTestScene();
        }
    },
    {
        title:"Path Tests",
        platforms: PLATFORM_ALL,
        linksrc:"src/PathTest/PathTest.js",
        testScene:function () {
            return new PathTestScene();
        }
    },
    {
        title:"Physics3D Test",
        platforms: PLATFORM_JSB,
        linksrc:"src/Physics3DTest/Physics3DTest.js",
        testScene:function () {
            return new Physics3DTestScene();
        }
    },
    {
        title:"NavMesh Test",
        platforms: PLATFORM_JSB,
        linksrc:"src/NavMeshTest/NavMeshTest.js",
        testScene:function () {
            return new nextNavMeshTest();
        }
    },
    {
        title:"ProgressActions Test",
        platforms: PLATFORM_ALL,
        linksrc:"src/ProgressActionsTest/ProgressActionsTest.js",
        testScene:function () {
            return new ProgressActionsTestScene();
        }
    },
    {
        title:"Reflection Test",
        platforms: PLATFROM_ANDROID | PLATFROM_APPLE,
        linksrc:"src/ReflectionTest/ReflectionTest.js",
        testScene:function () {
            return new ReflectionTestScene();
        }
    },
    {
        title:"RenderTexture Test",
        platforms: PLATFORM_ALL,
        linksrc:"src/RenderTextureTest/RenderTextureTest.js",
        testScene:function () {
            return new RenderTextureTestScene();
        }
    },
    {
        title:"RotateWorld Test",
        platforms: PLATFORM_ALL,
        linksrc:"src/RotateWorldTest/RotateWorldTest.js",
        testScene:function () {
            return new RotateWorldTestScene();
        }
    },
    {
        title:"Scene Test",
        platforms: PLATFORM_ALL,
        linksrc:"src/SceneTest/SceneTest.js",
        testScene:function () {
            return new SceneTestScene();
        }
    },
    {
        title:"Scheduler Test",
        platforms: PLATFORM_ALL,
        linksrc:"src/SchedulerTest/SchedulerTest.js",
        testScene:function () {
            return new SchedulerTestScene();
        }
    },
    {
        title:"Spine Test",
        resource: g_spine,
        platforms: PLATFORM_ALL,
        linksrc:"src/SpineTest/SpineTest.js",
        testScene:function () {
            return new SpineTestScene();
        }
    },
    {
        title:"Sprite3D Test",
        platforms: PLATFORM_JSB,
        linksrc:"src/Sprite3DTest/Sprite3DTest.js",
        testScene:function () {
            return new Sprite3DTestScene();
        }
    },
    {
        title:"SpritePolygon Test",
        platforms: PLATFORM_JSB,
        linksrc:"src/SpritePolygonTest/SpritePolygonTest.js",
        testScene:function () {
            return new SpritePolygonTestScene();
        }
    },
    {
        title:"Sprite Test",
        resource:g_sprites,
        platforms: PLATFORM_ALL,
        linksrc:"src/SpriteTest/SpriteTest.js",
        testScene:function () {
            return new SpriteTestScene();
        }
    },
    {
        title:"Scale9Sprite Test",
        resource:g_s9s_blocks,
        platforms: PLATFORM_ALL,
        linksrc:"src/ExtensionsTest/S9SpriteTest/S9SpriteTest.js",
        testScene:function () {
            return new S9SpriteTestScene();
        }
    },
    {
        title:"Terrain Test",
        platforms: PLATFORM_JSB,
        linksrc:"src/TerrainTest/TerrainTest.js",
        testScene:function () {
            return new TerrainTestScene();
        }
    },
    {
        title:"TextInput Test",
        platforms: PLATFORM_HTML5,
        linksrc:"src/TextInputTest/TextInputTest.js",
        testScene:function () {
            return new TextInputTestScene();
        }
    },
    //"Texture2DTest",
    {
        title:"TextureCache Test",
        platforms: PLATFORM_ALL,
        linksrc:"src/TextureCacheTest/TextureCacheTest.js",
        testScene:function () {
            return new TexCacheTestScene();
        }
    },
    {
        title:"TileMap Test",
        resource:g_tilemaps,
        platforms: PLATFORM_ALL,
        linksrc:"src/TileMapTest/TileMapTest.js",
        testScene:function () {
            return new TileMapTestScene();
        }
    },
    {
        title:"Touches Test",
        resource:g_touches,
        platforms: PLATFORM_HTML5,
        linksrc:"src/TouchesTest/TouchesTest.js",
        testScene:function () {
            return new TouchesTestScene();
        }
    },
    {
        title:"Transitions Test",
        resource:g_transitions,
        platforms: PLATFORM_ALL,
        linksrc:"",
        testScene:function () {
            return new TransitionsTestScene();
        }
    },
    {
        title:"Unit Tests",
        platforms: PLATFORM_ALL,
        linksrc:"src/UnitTest/UnitTest.js",
        testScene:function () {
            return new UnitTestScene();
        }
    },
    {
        title:"Sys Tests",
        platforms: PLATFORM_ALL,
        linksrc:"src/SysTest/SysTest.js",
        testScene:function () {
            return new SysTestScene();
        }
    },
    {
        title:"Vibrate Test",
        platforms: PLATFORM_JSB,
        linksrc:"src/VibrateTest/VibrateTest.js",
        testScene:function () {
            return new VibrateTestScene();
        }
    },
    {
        title:"cocos2d JS Presentation",
        platforms: PLATFORM_JSB,
        linksrc:"src/Presentation/Presentation.js",
        testScene:function () {
            return new PresentationScene();
        }
    },
    {
        title:"XMLHttpRequest",
        platforms: PLATFORM_ALL,
        linksrc:"src/XHRTest/XHRTest.js",
        testScene:function () {
            return new XHRTestScene();
        }
    },
    {
        title:"XMLHttpRequest send ArrayBuffer",
        platforms: PLATFORM_ALL,
        linksrc:"src/XHRTest/XHRArrayBufferTest.js",
        testScene:function () {
            return new XHRArrayBufferTestScene();
        }
    }

    //"UserDefaultTest",
    //"ZwoptexTest",
];
