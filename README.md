# IM SDK for Cocos2d js 使用指南

## IM SDK 常见问题->[IM FAQ详解](https://github.com/youmesdk/wiki/blob/master/IMFAQ.md)

## 导入IM SDK
### Android
- 将`yim`复制到Cocos2d-x开发工具生成的游戏目录根目录下。

- 拷贝yim下的两个文件：jsb_youmeim.h，jsb_youmeim.cpp到Classes目录。

- 对`proj.android/jni/Android.mk`文件进行如下修改，**仅需修改标注部分，其余部分无需处理。**

    ``` mk

    LOCAL_PATH := $(call my-dir)

    include $(CLEAR_VARS)

    LOCAL_MODULE := cocos2djs_shared

    LOCAL_MODULE_FILENAME := libcocos2djs

    ifeq ($(USE_ARM_MODE),1)
    LOCAL_ARM_MODE := arm
    endif

    ##########添加编译jsb_youmeim.cpp，注意上一行末尾添加‘\’####################
    LOCAL_SRC_FILES := hellojavascript/main.cpp \
              ../../Classes/AppDelegate.cpp \
                        ../../Classes/jsb_youmeim.cpp
    ################################################################

    ##########添加如下代码,添加头文件路径yim/include .注意上一行结尾处添加换行符\########
    LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../Classes  \
        $(LOCAL_PATH)/../../../../yim/include \
    ################################################################

    # _COCOS_HEADER_ANDROID_BEGIN
    # _COCOS_HEADER_ANDROID_END

    LOCAL_STATIC_LIBRARIES := cocos2d_js_static
    ##############添加如下代码,链接动态库##############################
    LOCAL_SHARED_LIBRARIES := YIM
    ################################################################

    LOCAL_EXPORT_CFLAGS := -DCOCOS2D_DEBUG=2 -DCOCOS2D_JAVASCRIPT

    include $(BUILD_SHARED_LIBRARY)
    ####################### 包含YMSDK子配置 #########################
    include $(LOCAL_PATH)/../../../../yim/lib/android/Android.mk
    ################################################################

    $(call import-module, scripting/js-bindings/proj.android)
    ```

- 若不需要语音转文字功能，需要对`yim/lib/android/Android.mk`文件进行如下修改：

    ``` mk
    LOCAL_PATH := $(call my-dir)

    include $(CLEAR_VARS)

    LOCAL_MODULE := YIM
    LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libyim.so \

    include $(PREBUILT_SHARED_LIBRARY)

    # ############# 不使用翻译可以删除下面内容 ##############
    include $(CLEAR_VARS)

    LOCAL_MODULE := YIMM
    LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libmsc.so \
                           
    include $(PREBUILT_SHARED_LIBRARY)
    
    include $(CLEAR_VARS)
    LOCAL_MODULE := YIMNLS
    LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libnlscppsdk.so \                       
                       
    include $(PREBUILT_SHARED_LIBRARY)
    
    include $(CLEAR_VARS)
    LOCAL_MODULE := YIMUSC
    LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libuscasr.so \
                       
    include $(PREBUILT_SHARED_LIBRARY)
    
    include $(CLEAR_VARS)
    LOCAL_MODULE := YIMU
    LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)+/libuuid.so \
                                             
    include $(PREBUILT_SHARED_LIBRARY) 
    # ###################################################
    ```

- 将 `yim/lib/android/` 目录中如下文件复制到`proj.android/libs/`目录下。  
>`yim.jar`.  
>`Msc.jar`.  
>`alisr.jar`.  
>`usc.jar`.  
>`Sunflower.jar`.  
>`android-support-v4.jar`.  


-  修改`proj.android/AndroidManifest.xml`文件，确保声明了如下权限：

``` xml
  <!-- 添加到application节点内 -->
  <application xxxx>
      <receiver android:name="com.youme.im.NetworkStatusReceiver" android:label="NetworkConnection" >
        <intent-filter>
            <action android:name="android.net.conn.CONNECTIVITY_CHANGE" />
        </intent-filter>
    </receiver>
  </application>
  <!-- 添加到跟application平级 -->
  <uses-permission android:name="android.permission.INTERNET" />
  <uses-permission android:name="android.permission.ACCESS_NETWORK_STATE" />
  <uses-permission android:name="android.permission.ACCESS_WIFI_STATE" />
  <uses-permission android:name="android.permission.CHANGE_WIFI_STATE" />
  <uses-permission android:name="android.permission.CHANGE_NETWORK_STATE" />
  <uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE" />
  <uses-permission android:name="android.permission.READ_PHONE_STATE" />
  <uses-permission android:name="android.permission.RECORD_AUDIO" />
  <!-- 获取地理位置的权限，可选 -->
  <uses-permission android:name="android.permission.ACCESS_FINE_LOCATION" />
  <uses-permission android:name="android.permission.ACCESS_COARSE_LOCATION" />
```

- 打开Eclipse，导入上一步的Android工程，在项目中第一个启动的`AppActivity`中的`onCreate`中增加以下Java代码(默认在项目根目录／src目录下)：
    `注意必须把游密的初始化加到第一行，否则可能出现部分机型加载so不正常`

    ``` java
    /**
    如果没有onCreate方法，可以添加到onCreateView方法里
    */

    @Override
    protected void onCreate(Bundle bundle)
    {
       //下面的两个调用顺序不能错
        com.youme.im.IMEngine.init(this);
        super.onCreate(bundle);           //super.onCreate是默认已经有的，不用再复制
    }
    ```

-  如果打包或生成APK时需要混淆，则需要在`proguard.cfg`文件中添加如下代码：

    ``` shell
    -keep class com.youme.**{*;}
    -keep class com.youme.**{*;}
    -keep class com.iflytek.**
    -keepattributes Signature
    ```

### iOS
- 将`yim`复制到Cocos2d-x开发工具生成的游戏目录**根目录下**
- 在`Build Settings -> Search Paths -> Header Search Paths`中添加头文件路径：`$(SRCROOT)/../../../yim/include/`  
- 在`Build Settings -> Search Paths -> User Header Search Paths`中添加头文件路径：`$(SRCROOT)/../../cocos2d-x/cocos/scripting/js-bindings/manual`  
- 在`Build Settings -> Search Paths -> Framework Search Paths`中添加框架文件路径：`$(SRCROOT)/../../../yim/lib/ios`  
- 在`Build Settings -> Search Paths -> Library Search Paths`中添加库文件路径：`$(SRCROOT)/../../../yim/lib/ios`  
- 在`Build Phases  -> Link Binary With Libraries`添加如下依赖库：  
 >`libc++.1.tbd`  
 >`libsqlite3.0.tbd`  
 >`libresolv.9.tbd`  
 >`SystemConfiguration.framework`  
 >`libYouMeCommon.a`  
 >`libyim.a`  
 >`libz.tbd`  
 >`CoreTelephony.framework`  
 >`AVFoundation.framework`  
 >`AudioToolbox.framework`   
 >`CoreLocation.framework`    
 >`iflyMSC.framework`       //如果是带语音转文字的sdk需要添加.  
 
 >`AliyunNlsSdk.framework`  //如果是带语音转文字的sdk需要添加，该库是动态库，需要在`General -> Embedded Binaries`也进行添加。  
 
>`USCModule.framework`     //如果是带语音转文字的sdk需要添加.  

 

- 拷贝yim下的两个文件：jsb_youmeim.h，jsb_youmeim.cpp到Classes目录，并加入项目。

- **需要为iOS10以上版本添加录音权限配置**
iOS 10 使用录音权限，需要在`info`新加`Privacy - Microphone Usage Description`键，值为字符串，比如“语音聊天需要录音权限”。
首次录音时会向用户申请权限。配置方式如图(选择Private-Microphone Usage Description)。
![iOS10录音权限配置](https://www.youme.im/doc/images/im_iOS_record_config.jpg)

- **LBS添加获取地理位置权限**
若使用LBS，需要在`info`新加`Privacy - Location Usage Description`键，值为字符串，比如“查看附近的玩家需要获取地理位置权限”。首次使用定位时会向用户申请权限，配置方式如上图录音权限。

### Windows
- 将`yim`复制到Cocos2d-x开发工具生成的游戏目录<b>根目录下</b>。

- 拷贝yim下的两个文件：jsb_youmeim.h，jsb_youmeim.cpp到Classes目录,并加入项目。

- 拷贝yim/lib/x86 目录下的` msc.dll， yim.dll， yim.lib， libusc.dll， jsoncpp-0.y.z.dll， libeay32.dll， nlscppsdk.dll， pthreadVC2.dll， ssleay32.dll`拷贝到执行目录(如果是64位机器，使用yim/lib/x64)

- 在项目 -> 属性 -> C/C++ -> 常规 -> 附加包含目录 中添加
  `$(ProjectDir)..\..\..\yim\include`

  `$(EngineRoot)cocos\scripting\js-bindings\manual`

- 在项目->属性->链接器->输入->附加依赖库 中添加 ` yim.lib`（该库是release模式下的  如果运行的时候是debug模式请添加yim_Debug.lib）

## 特别注意事项
* 重连有可能失败，所以一般收到OnLogout的通知后需要判断游戏是否还在线，如果游戏还在线，就重新调用 登录->进频道 接口
* 不同设备或者不同的运行期，消息id可能重复
* 返回值为ErrorCode的接口，如果返回了非YIMErrorcode_Success表示接口调用失败，就不会再有回调通知（如果是有回调的接口）
* 请务必使用游密SDK提供的`android-support-v4.jar` ，这样可以兼容`targetSdkVersion="23"`及其以上版本的录音授权模式

## 基本接口使用时序图
![基本接口使用时序图](https://youme.im/doc/images/im_sdk_flow_client_csharp.png)

## 初始化
### 注册接口
- 引入C++接口文件
`#include "jsb_youmeim.h"`
- 注册js方法
![regjs.png](https://youme.im/doc/images/im_sdk_lua-2.jpg)

### js实现参考
`tests-main.js`


### 获取SDK管理器
**为了方便说明如何调用接口，我们全文中都把youmeim作为YouMeIM的实例对象使用**

- 接口和示例：

  ``` javascript
      var youmeim = new YouMeIM
  ```

### 初始化SDK 
初始化的函数主要功能是初始化IM SDK引擎。初始化接口的输入参数`strAppKey`和`strAppSecret`需要根据实际申请得到的值进行替换, `strPackageName`目前传入空字符串即可。

- 接口示例：

  ``` javascript
      var errorcode = youmeim.IM_Init("strAppKey","strAppSecret")
  ```
  
- 参数：
  `strAppKey`：用户游戏产品区别于其它游戏产品的标识，可以在[游密官网](https://account.youme.im/login)获取、查看
  `strAppSecret`：用户游戏产品的密钥，可以在[游密官网](https://account.youme.im/login)获取、查看  
  
- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，详细描述见[错误码定义](#错误码定义)

### 设置监听 
初始化之后需要立即设置回调处理方法。SDK内部所有的较长耗时的接口调用都会采用异步回调的方式返回结果，所以需要开发者实现:

- 接口：

``` javascript
//登录登出回掉
youmeim.OnLogin = function (errorcode,youmeID)
{
  cc.log("errorcode:" + errorcode + " youmeID:" + youmeID)
}

youmeim.OnLogout = function ()
{
  cc.log("11: OnLogout")
}

//下载语音文件回调
youmeim.OnDownload = function( errorcode, strSavepath, bodytype,chattype,serial,recvid,senderid,content,params,duration,createtime ){
{
  cc.log("OnDownload：" + serial + " errorcode:" + errorcode + " path:" + strSavepath)
}

//从URL下载文件回调
youmeim.OnDownloadByUrl = function ( errorcode , fromUrl, savePath ){
    cc.log("OnDownload：" + fromUrl + " errorcode:" + errorcode + " path:" + strSavepath)
}

//文本和自定义消息发送状态回掉
youmeim.OnSendMessageStatus = function(serial,errorcode, sendTime, isForbidRoom, reasonType, endTime)
{
  cc.log("OnSendMessageStatus" + serial + " errorcode:" + errorcode)
}

//自己的语音开始上传回调
youmeim.OnStartSendAudioMessage = function(serial,errorcode,content,localpath,duration)
{
  cc.log("OnStartSendAudioMessage" + serial + " errorcode:" + errorcode + " content:" + content + " localpath:" + localpath +" duration:" + duration)
}

//自己的语音消息发送状态回调
youmeim.OnSendAudioMessageStatus = function(serial,errorcode,content,localpath,duration, sendTime, isForbidRoom, reasonType, endTime)
{
  cc.log("OnSendAudioMessageStatus" + serial + " errorcode:" + errorcode + " content:" + content + " localpath:" + localpath +" duration:" + duration)
}

//接收到他人的消息回调
youmeim.OnRecvMessage = function(bodytype,chattype, serial,recvid,senderid,content,params,duration)
{
  cc.log("OnRecvMessage" + bodytype + " chattype:" + chattype + " serial:" + serial + " recvid:" + recvid + " senderid:" + senderid + " content:" + content + " param:" + params )
  if(bodytype==1){
    //文本消息
  }else if(bodytype==5){
    //语音消息
  }
}

//进入聊天频道的通知
youmeim.OnJoinChatroom = function(errorcode,groupid)
{
  cc.log("OnJoinChatroom:" + groupid + " errorcode:" + errorcode)
}
// 离开聊天室通知
youmeim.OnLeaveChatroom = function(errorcode,groupid)
{
  cc.log("OnLeaveChatroom:" + groupid + " errorcode:" + errorcode)
}
// 收到新消息的通知
youmeim.OnRecvMessageNotify = function( chattype, targetID )
{
  cc.log("OnRecvMessageNotify:" + chattype + " , " + targetID )
}
// 获取录音文件地址的通知
youmeim.OnAudioSpeech = function(serial,errorcode,strDownloadURL,iDuration,iFileSize,strLocalPath,strText)
{
  cc.log("OnAudioSpeech" + serial + " errorcode:" + errorcode + " url:" + strDownloadURL + "Duration:" + iDuration + "FileSize:" + iFileSize + "LocalPath:" + strLocalPath + "text:" + strText)
}
// 最近联系人查询结果通知
youmeim.OnGetContactList = function(errorcode,contactLists)
{
  for(var i=0;i<contactLists.length;i++)
  {
    cc.log("OnGetContactList " + contactLists[i])
  }
}
// 私聊消息历史记录查询通知
youmeim.OnQueryHistoryMsg = function(targetID,remainCount,msgLists)
{
  for(var i=0;i<msgLists.length;i++)
  {
    cc.log("OnQueryHistoryMsg： chattype：" + msgLists[i].ChatType + "Duration:" + msgLists[i].Duration + "|" + msgLists[i].MessageType + "|" +  msgLists[i].Param + "|" +  msgLists[i].ReceiveID + "|" +  msgLists[i].SenderID+  msgLists[i].Serial+  msgLists[i].Text + msgLists[i].LocalPath)
  }

}
//获取指定玩家信息回调
youmeim.OnGetUserInfo = function( errorcode, jsonUserInfo ){
    cc.log("获得信息回调:%d", errorcode );
    var obj = JSON.parse( jsonUserInfo );
    cc.log( obj.nickname );
    cc.log( obj.server_area );
    cc.log( obj.location );
    cc.log( obj.level );
    cc.log( obj.vip_level );
    cc.log( obj.extra );
};
youmeim.OnQueryUserStatus = function ( errorcode, userid, state  )
{
    cc.log( "查询用户登录状态, userid:" +userid +"_" + state );
    //通知有新消息，只有在手动接受消息的模式下才有用
};

youmeim.OnPlayCompletion = function ( errorcode, path  ){
    cc.log( "播放结束:"+path);
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
youmeim.OnGetNearbyObjects = function(errorcode, neighbourList, startDistance, endDistance){
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

// 开始重连回调
youmeim.OnStartReconnect = function(){
    cc.log("OnStartReconnect");
};

// 重连结果回调
youmeim.OnRecvReconnectResult = function(result){
    cc.log("OnRecvReconnectResult result:"+ result);
};
```

### 重连回调 

- 开始重连回调：
  
  ``` js
      youmeim.OnStartReconnect=function() {}  
  ```
  
- 重连结果回调：
  
  ``` js
      youmeim.OnRecvReconnectResult=function(result) {}  
  ```
  
- 参数:
  `result `：重连结果，0-重连成功,1-重连失败，再次重连,
             2-重连失败

### 录音音量回调 
音量值范围:0~1, 频率:1s ios 约2次，android 约8次
  
- 音量回调通知：
  
  ``` js
  youmeim.OnRecordVolume=function(volume) {}
  
  ```
  
- 参数：
  `volume `：音量值  

## 用户管理
### 用户登录 
完成以上的步骤后就可以使用IM功能了，IM用户登录IM后台服务器后即可以正常收发消息。
登录接口需要用户提供用户名、密码。登录为异步过程，通过回调函数返回是否成功，成功后方能进行后续操作。
用户首次登录会自动注册，自行设置登录用户名和登录密码，如果后台已存在此用户名，则会提示校验密码（用户名和密码的格式见下方相关参数说明）。

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_Login("userid","user_password","token")
  ```
  
- 参数说明：
  `userid `：用户ID，由调用者分配，不可为空字符串，只可由字母或数字或下划线组成，长度限制为255字节
  `user_password`：用户密码，不能为空，一般固定为"123456"即可
  `token`：使用服务器token验证模式时使用该参数，否则使用空字符串：""，由restAPI获取token值

- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，失败后不会有回调通知，详细描述见[错误码定义](#错误码定义)
  
- 回调通知：
 
  ``` js
  youmeim.OnLogin = function(errorcode,userid) {}
  ```  
 
- 回调参数：
  `errorcode`：错误码
  `userid`：用户ID

### 用户登出  
如用户主动退出或需要进行用户切换，则需要调用登出操作，登出接口为`IM_Logout`。

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_Logout()
  ```
  
- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，失败后不会有回调通知，详细描述见[错误码定义](#错误码定义)
  
- 回调通知：
 
  ``` js
  youmeim.OnLogout = function() {}
  ```  

### 被用户踢出通知
同一个用户ID在多台设备上登录时，后登录的会把先登录的踢下线，收到OnKickOff通知。

- 被踢出通知：

``` js
youmeim.OnKickOff = function() {}
```
 
### 用户信息
#### 设置用户的详细信息 
- 接口示例：

  ``` js
      var errorcode = youmeim.IM_SetUserInfo( strUserInfo )
  ```

- 参数：
  `strUserInfo`：字符串,用户信息的json串。包含字段：    `nickname`,`server_area_id`,`server_area`,`location_id`,`location`,`level`,`vip_level`,`extra`
  `nickname`：用户昵称
  `server_area_id`：游戏服ID
  `server_area`：游戏服名称
  `location_id`：游戏大区ID
  `location`：游戏大区名  
  `level`：角色等级
  `vip_level`：玩家VIP等级
  `extra`：其它自定义扩展信息，建义用json字符串
  
- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，详细描述见[错误码定义](#错误码定义)  

#### 获取指定用户的详细信息  
- 接口示例：

  ``` js
      var errorcode = youmeim.IM_GetUserInfo( strUserID  )
  ```
  
- 参数：
  `strUserID`：字符串,用户的ID（登录IM，发送消息时用的ID）
  
- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，失败后不会有回调通知，详细描述见[错误码定义](#错误码定义)  

- 回调通知：

  ``` js
  youmeim.OnGetUserInfo = function( errorcode, userid, strUserInfo ){}
  ```
  
- 回调参数：
  `errorcode`：错误码
  `userid`：用户ID
  `strUserInfo` :用户详细信息,json串，包含以下字段`nickname`,`server_area_id`,`server_area`,`location_id`,`location`,`level`,`vip_level`,`extra`

#### 查询用户在线状态  
- 接口示例：

  ``` js
      var errorcode = youmeim.IM_QueryUserStatus(strUserID)
  ```
  
- 参数：
  `strUserID`：字符串,用户的ID（登录IM，发送消息时用的ID）
  
- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，失败后不会有回调通知，详细描述见[错误码定义](#错误码定义)  

- 回调通知：

  ``` js
  youmeim.OnQueryUserStatus = function( errorcode, userid, state  ) {}
  ```  

- 回调参数：
  `errorcode`：错误码
  `userid`：查询的用户ID
  `state`：登录状态，0表示在线，1表示不在线

## 频道管理
### 加入频道 
通过`IM_JoinChatRoom`接口加入聊天频道，如果频道不存在则后台自动创建。有了这个ID就可以收发频道消息。开发者要保证频道号全局唯一，以避免用户进入错误频道。

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_JoinChatRoom(room_id)
  ```

- 参数：
  `room_id `：请求加入的频道ID，仅支持数字、字母、下划线组成的字符串，区分大小写，长度限制为255字节

- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，失败后不会有回调通知，详细描述见[错误码定义](#错误码定义)

- 回调通知：

  ``` js
  youmeim.OnJoinChatroom=function(errorcode,channelID){}
  ```
  
- 回调参数：
  `errorcode`：错误码
  `channelID`：频道ID   

### 离开频道 
通过`IM_LeaveChatRoom`接口离开频道。

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_LeaveChatRoom(room_id)
  ```

- 参数：
  `room_id `：请求离开的频道ID，字符串

- 返回值：
  `errorcode`：`0`为调用成功，`非0`表示调用失败，失败后不会有回调通知，详细描述见[错误码定义](#错误码定义)
  
- 回调通知：

  ``` js
  youmeim.OnLeaveChatroom=function(errorcode,channelID){}
  ```
  
- 回调参数：
  `errorcode`：错误码
  `channelID`：频道ID  
  
### 离开所有频道 
通过`IM_LeaveAllChatRooms`接口离开所有已进入的频道。

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_LeaveAllChatRooms()
  ```
    
- 返回值：
  `errorcode`：`0`为调用成功，`非0`表示调用失败，失败后不会有回调通知，详细描述见[错误码定义](#错误码定义) 
  
- 回调通知：

  ``` js
  youmeim.OnLeaveAllChatRooms=function(errorcode){}
  ```
  
- 回调参数：
  `errorcode`:  错误码
  
### 用户进出频道通知
小频道(小于100人)内，当其他用户进入频道，会收到`OnUserJoinChatRoom`通知

- 进入频道通知：

  ``` js
      youmeim.OnUserJoinChatRoom = function(channelID, userID)
  ```

- 参数：
  `channelID `：频道ID
  `userID`：用户ID

小频道(小于100人)内，当其他用户退出频道，会收到`OnUserLeaveChatRoom`通知

- 离开频道通知：

  ``` js
      youmeim.OnUserLeaveChatRoom = function(channelID, userID)
  ```

- 参数：
  `channelID `：频道ID
  `userID`：用户ID

### 获取房间成员数量 
获取进入频道的用户数量。

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_GetRoomMemberCount(chatRoomID);
  ```
  
- 参数：
  `chatRoomID `：频道ID(已成功加入此频道才能获取该频道的人数)，字符串
  
- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，失败后不会有回调通知，详细描述见[错误码定义](#错误码定义) 

- 回调通知：

  ``` js
  OnGetRoomMemberCount = function(errorcode, roomID, count)
  ```
    
- 回调参数：
  `errorcode `：错误码
  `roomID`：频道ID
  `count`：频道中的成员数量

## 消息管理
### 接收消息  
通过`OnRecvMessage`接口被动接收消息，需要开发者实现。

- 接收消息通知：

  ``` js
  youmeim.OnRecvMessage = function(bodyType,chatType,serial,recvID,senderID,content,param,duration,createTime,giftID,giftCount,anchorID) {}
  ```

- 参数：
  `bodyType`： 消息类型，0-未知类型，1-文本消息，2-自定义消息，3-表情，4-图片，5-语音，6-视频，7-文件，8-礼物  
  `chatType`：聊天类型，私聊/频道聊天
  `serial`： 消息ID
  `recvID`： 消息接收者ID
  `senderID`： 消息发送者ID
  `content`： 文本消息内容/自定义消息内容/语音识别出的文本
  `param`：附加参数（语音消息，礼物消息，文本消息）
  `duration`：语音时长（单位：秒）
  `createTime`：消息发送时间
  `giftID`：礼物ID
  `giftCount`：礼物数量
  `anchorID`：主播ID
      
### 发送文本消息 
可通过`IM_SendTextMessage`接口发送消息。异步返回结果通过`OnSendMessageStatus`接口返回。

- 接口示例：

  ``` js
      var requestID = youmeim.IM_SendTextMessage(str_reciver_id,iChatType,message_content, attachParam);
  ```

- 参数：
  `str_reciver_id `：接收者ID（用户ID或者频道ID），字符串
  `iChatType`：整型，1表示私聊，2表示频道聊天
  `message_content`：聊天内容
  `attachParam`：发送文本附加信息

- 返回值：
  `requestid`：消息序列号,字符串类型（实际是长整型数字），`-1`表示调用失败，失败后不会有回调通知。
  
- 回调通知：
 
  ``` js
  youmeim.OnSendMessageStatus = function(serial,errorcode,sendTime,isForbidRoom,reasonType,forbidEndTime) {}
  ```  
  
- 回调参数：
  `serial`：消息序列号，用于校验一条消息发送成功与否的标识
  `errorcode`：错误码
  `sendTime`：消息发送时间
  `isForbidRoom`：若发送的是频道消息，显示在此频道是否被禁言，true-被禁言，false-未被禁言，（errorcode==ForbiddenSpeak(被禁言)才有效）
  `reasonType`：若在频道被禁言，禁言原因类型，0-未知，1-发广告，2-侮辱，3-政治敏感，4-恐怖主义，5-反动，6-色情，7-其它
  `forbidEndTime` ：若在频道被禁言，禁言结束时间 
  
### 群发文本消息 
用于群发文本消息的接口，**每次不要超过200个用户**。

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_MultiSendTextMessage(userids,text)
  ```
  
- 参数：
  `userids`：接受消息的用户id数组,比如：`["123","3456","43sdf"]`
  `text`：文本消息内容，比如：`"text content"`
  
- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，详细描述见[错误码定义](#错误码定义) 

### 发送礼物消息 
给主播发送礼物消息的接口，支持在游密主播后台查看礼物消息信息和统计信息。客户端还是通过`OnRecvMessage`接收消息。

- 接口示例：

  ``` js
      var requestid = youmeim.IM_SendGift(strAnchorId, strChannel, intGiftId, intGiftCount, strExtraParam)
  ```

- 参数：
  `strAnchorId`：游密后台设置的对应的主播游戏id，字符串
  `strChannel`：主播频道id，通过`JoinChatRoom`进入频道的用户可以接收到消息，字符串。
  `intGiftId`：礼物物品id
  `intGiftCount`：礼物物品数量
  `strExtraParam`：扩展内容json字符串，目前要求包含如下字段：
`{"nickname":"","server_area":"","location":"","score":"","level":"","vip_level":"","extra":""}`

- 返回：
  `requestid`：消息序列号,字符串类型（实际是长整型数字），`-1`表示调用失败，失败后不会有回调通知。
  
- 回调通知：
 
  ``` js
  youmeim.OnSendMessageStatus = function(serial,errorcode,sendTime,isForbidRoom,reasonType,forbidEndTime) {}
  ```  
  
- 回调参数：
  `serial`：消息序列号，用于校验一条消息发送成功与否的标识
  `errorcode`：错误码
  `sendTime`：消息发送时间
  `isForbidRoom`：若发送的是频道消息，显示在此频道是否被禁言，true-被禁言，false-未被禁言，（errorcode==ForbiddenSpeak(被禁言)才有效）
  `reasonType`：若在频道被禁言，禁言原因类型，0-未知，1-发广告，2-侮辱，3-政治敏感，4-恐怖主义，5-反动，6-色情，7-其它
  `forbidEndTime` ：若在频道被禁言，禁言结束时间   

### 语音消息管理
**语音消息聊天简要流程：**

-  调用IM的语音发送接口就开始录音，调用结束录音接口后自动停止录音并发送。
-  接收方接收语音消息通知后，调用方控制是否下载，调用下载接口就可以获取录音内容。

接收方接收语音消息通知后，调用方控制是否下载，调用下载接口就可以获取录音内容。然后开发者调用播放接口播放wav音频文件。

#### 设置是否自动下载语音消息 
IM_SetDownloadAudioMessageSwitch()在初始化之后，启动语音之前调用;若设置了自动下载语音消息，不需再调用IM_DownloadAudioFile()接口，收到语音消息时会自动下载，自动下载完成也会收到IM_DownloadAudioFile()接口对应的OnDownload()回调。

- 接口示例：

  ``` js
      var errorcode = IM_SetDownloadAudioMessageSwitch(download);
  ```
  
- 参数：
  `download `：true-自动下载语音消息，false-不自动下载语音消息(默认) 
  
- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，详细描述见[错误码定义](#错误码定义) 

#### 设置语音识别的语言 
`IM_SendAudioMessage`提供语音识别功能，将语音识别为文字，默认输入语音为普通话，识别文字为简体中文，可通过`IM_SetSpeechRecognizeLanguage`函数设置语言；若需要识别为繁体中文，需要联系我们配置此服务。

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_SetSpeechRecognizeLanguage(language)
  ```

- 参数：
  `language`：语言（0-普通话 1-粤语 2-四川话 3-河南话 4-英语 5-繁体中文）
  
- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，详细描述见[错误码定义](#错误码定义)   

#### 发送语音消息
##### 启动录音 
调用`IM_SendAudioMessage`语音发送接口（该接口支持语音转文字，若不需要转文字建议使用`IM_SendOnlyAudioMessage`）就开始录音，调用`IM_StopAudioMessage`后自动停止录音并发送。调用`IM_CancleAudioMessage`取消本次消息发送。注意：语音消息最大的时长是1分钟（超过1分钟就自动发出去）

- 接口示例：

  ``` js
      // ========== 带文字识别的录音接口 ============
      var requestID = youmeim.IM_SendAudioMessage(reciver_id,chatType)
      
      // ========== 不带文字识别的录音接口 ============
      var requestID = youmeim.IM_SendOnlyAudioMessage(reciver_id,chatType)
  ```
  
- 参数：
  `reciver_id`：接收者ID，私聊传入userid，频道聊天传入roomid，字符串
  `chatType`：整型，1表示私聊，2表示频道聊天

- 返回值：
  `requestid`：消息序列号,字符串类型（实际是长整型数字），`-1`表示调用失败，失败后不会有回调通知。

##### 结束录音并发送 

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_StopAudioMessage(extra_param)
  ```
  
- 参数：
  `extra_param`：给语音消息附加自定义参数，比如json字符串，可为空字符串

- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，失败后不会有回调通知，详细描述见[错误码定义](#错误码定义)
  
- 异步回调接口：
  结束录音并发送接口对应两个回调接口，若语音成功发送出去能得到两个回调通知，语音发送失败则只会得到`发送语音结果回调`。
  
  ``` js
  // 开始上传语音回调，（调用IM_StopAudioMessage停止语音之后，成功发送语音消息之前），录音结束，开始发送录音的通知，这个时候已经可以拿到语音文件进行播放
  youmeim.OnStartSendAudioMessage = function(serial,errorcode,content,localpath,duration){}
  
  // 发送语音结果回调，自己的语音消息发送成功或者失败的通知。
  youmeim.OnSendAudioMessageStatus = function(serial, errorcode, content, localpath, duration, sendTime, isForbidRoom, reasonType, endTime){}
  ```  
  
- 参数：
  `serial`：消息序列号，用于校验一条消息发送成功与否的标识
  `errorcode`：错误码，等于0才是操作成功。
  `content`：语音转文字识别的文本内容，如果没有用带语音转文字的接口，该字段为空字符串
  `localpath`：录音生成的wav文件的本地完整路径，比如"/sdcard/xxx/xxx.wav"
  `duration`：录音时长（单位为秒）  
  
  `sendTime`：发送时间
  `isForbidRoom`：是否房间被禁言，1-房间被禁言，0-玩家被禁言（errorcode为禁言才有效）
  `reasonType`：禁言原因，参见ForbidSpeakReason（errorcode为禁言才有效）
  `forbidEndTime`：禁言截止时间戳（errorcode为禁言才有效）

##### 取消本次录音 

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_CancleAudioMessage()
  ```
  
- 参数：
  无。

- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，详细描述见[错误码定义](#错误码定义)

#### 接收语音消息 
接收消息接口在OnRecvMessage回调，通过`bodytype == 5`分拣出语音消息，用`serial`来下载语音文件，调用函数`IM_DownloadAudioFile`下载语音消息，下载成功会通知`OnDownload`
**windows下，会对下载接口的保存路径参数的'/'转换为'\'；如果传入的保存路径参数不符合windows下的路径格式，下载回调中的保存路径可能和传入的保存路径不同**

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_DownloadAudioFile(serial,save_path)
  ```
  
- 参数：
  `serial`：消息ID,数字字符串
  `save_path`：指定文件保存路径(带文件名的全路径)，比如"/sdcard/cache/1.wav"，如果目录不存在，SDK会自动创建，字符串

- 返回值：
  `errorcode`: `0`为调用成功，`非0`表示调用失败，失败后不会有回调通知，详细描述见[错误码定义](#错误码定义)
  
- 回调通知：

  ``` js
  youmeim.OnDownload = function(errorcode,savePath,bodyType,chatType,serial,recvID,senderID,content,param,audioTime,createTime) {}
  ```  
  
- 回调参数：
  `errorcode`：下载结果错误码
  `savePath`：保存路径 
  `bodyType`：消息类型，0-未知类型，1-文本消息，2-自定义消息，3-表情，4-图片，5-语音，6-视频，7-文件，8-礼物
  `chatType`：聊天类型，私聊/频道聊天
  `serial`：消息ID
  `recvID`：消息接收者ID
  `senderID`：消息发送者ID
  `content`：语音识别的文本内容
  `param`：发送语音消息的附加信息
  `audioTime`：语音时长（单位：秒）
  `createTime`：消息发送时间
    
#### 语音播放
##### 播放语音 
- 接口示例：

  ``` js
      var errorcode = youmeim.IM_StartPlayAudio( path)
  ```
  
- 参数：
  `path`：语音文件的绝对路径，字符串
  
- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，失败后不会有回调通知，详细描述见[错误码定义](#错误码定义)  

- 回调通知：

  ``` js
  //播放完成
  youmeim.OnPlayCompletion = function ( errorcode, path) {}
  ```
  
- 回调参数：
  `errorcode`：错误码
  `path`：被播放的音频文件地址 

##### 停止语音播放  

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_StopPlayAudio()
  ```
  
- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，详细描述见[错误码定义](#错误码定义) 
    
##### 设置语音播放音量 
设置语音播放的音量大小。

- 接口示例：

  ``` js
      youmeim.IM_SetVolume( volume )
  ```
  
- 参数：
  `volume`:音量值，取值范围`0.0f` - `1.0f`，默认值为 `1.0f`。
       
##### 查询播放状态 
查询当前音频播放器的状态。

- 接口示例：

  ``` js
      var res = youmeim.IM_IsPlaying()
  ```
  
- 返回值：
  `res`: 1-正在播放，0-没有在播放
    
#### 录音缓存
##### 设置录音缓存目录 
设置录音时用于保存录音文件的缓存目录，如果没有设置，SDK会在APP默认缓存路径下创建一个文件夹用于保存音频文件。
**该接口建议初始化之后立即调用** 。

- 接口示例：

  ``` js
      youmeim.IM_SetAudioCacheDir(dir_path)
  ```
  
- 参数：
  `dir_path`：缓存录音文件的文件夹路径，如果目录不存在，SDK会自动创建，字符串
  
- 返回值：
  无。

##### 获取当前设置的录音缓存目录 
获取当前设置的录音缓存目录。

- 接口示例：

  ``` js
      var path = youmeim.IM_GetAudioCacheDir()
  ```

- 返回值：
  返回当前设置的录音缓存目录的完整路径

##### 清空录音缓存目录 
清理语音缓存目录(注意清空语音缓存目录后历史记录中会无法读取到音频文件，调用清理历史记录接口也会自动删除对应的音频缓存文件)

- 接口示例：

  ``` js
      var res = youmeim.IM_ClearAudioCacheDir();
  ```
  
- 返回值：
  `res`: 1-清理成功，0-清理失败  
    
#### 设置只识别语音文字 
此功能是为了只获取语音识别的文字，不发送语音消息，接口调用与发送语音消息相同，但在成功录音后仅会收到识别的语音文本通知。
实现只识别语音文字功能的接口调用顺序：IM_SetOnlyRecognizeSpeechText->IM_SendAudioMessage->IM_StopAudioMessage->OnGetRecognizeSpeechText

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_SetOnlyRecognizeSpeechText(recognition);
  ```
  
- 参数：
  `recognition `：`1`为只识别语音文字，`0`为识别语音文字并发送语音消息
  
- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，详细描述见[错误码定义](#错误码定义)

- 回调通知：

  ``` js
  youmeim.OnGetRecognizeSpeechText = function (serial, errorcode, text) {}
  ```
  
- 回调参数：
  `serial `：消息ID
  `errorcode `：错误码
  `text `：返回的语音识别文本内容
    
#### 获取麦克风状态 
这是一个异步操作，操作结果会通过回调参数返回。（该接口会临时占用麦克风）
- 接口示例：

  ``` js
      youmeim.IM_GetMicrophoneStatus();
  ```

- 回调通知：

  ``` js
  youmeim.OnGetMicrophoneStatus = function(status)
  ```
  
- 回调参数：
  `status`：麦克风状态值，0-可用，1-无权限，2-静音，3-不可用      
    
### 录音上传
#### 启动录音 
该接口启动的录音

- 接口示例：

  ``` js
      var requestID = youmeim.IM_StartAudioSpeech(translate)
  ```

- 参数：
  `translate`：是否进行语音转文字识别，`0`表示不需要文字识别，`1`表示需要语音转文字

- 返回值：
  `requestid`：消息序列号,字符串类型（实际是长整型数字），`-1`表示调用失败，失败后不会有回调通知。

#### 停止录音并上传 
该接口只上传到服务器并异步返回音频文件的下载链接，不会自动发送，该接口对应`IM_StartAudioSpeech()`。

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_StopAudioSpeech()
  ```
  
- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，失败后不会有回调通知，详细描述见[错误码定义](#错误码定义)  

- 相关接口：

  ``` js
      -- 取消本次录音
      var errorcode = youmeim.IM_CancleAudioMessage()
  ```

- 回调通知：

  ``` js    
  youmeim.OnAudioSpeech = function(serial,errorcode,strDownloadURL,iDuration,iFileSize,strLocalPath,strText)
 {
  cc.log("OnAudioSpeech" + serial + " errorcode:" + errorcode + " url:" + strDownloadURL + "Duration:" + iDuration + "FileSize:" + iFileSize + "LocalPath:" + strLocalPath + "text:" + strText)
 }
 ```
 
- 回调参数：
  `serial`：消息ID
  `errorcode`：错误码
  `strDownloadURL`：语音文件的下载地址，amr格式
  `iDuration`：录音时长，单位秒
  `iFileSize`：文件大小，字节
  `strLocalPath`：本地语音文件的路径（wav格式）
  `strText` ：语音识别结果，可能为空字符串

#### 根据Url下载录音文件 

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_DownloadFileByUrl(strDownloadURL,save_path);
  ```

- 参数：
  `strDownloadURL`：语音文件的url地址，OnAudioSpeech返回的strDownloadURL参数，字符串
  `save_path`：下载语音文件的本地存放地址,带文件名的全路径，字符串
  
- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，失败后不会有回调通知，详细描述见[错误码定义](#错误码定义)  

- 回调通知：

  ```js
  youmeim.OnDownloadByUrl = function ( errorcode , fromUrl, savePath ) {}
  ```
  
- 回调参数：
  `errorcode`：错误码
  `fromUrl`：下载的语音文件url
  `savePath`：本地存放地址  
      
### 消息屏蔽
#### 屏蔽/解除屏蔽用户消息 
若屏蔽用户的消息，此屏蔽用户发送的私聊/频道消息都接收不到。

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_BlockUser(userID, block)
  ```
  
- 参数：
  `userID `：需屏蔽/解除屏蔽的用户ID，字符串
  `block `：true-屏蔽 false-解除屏蔽
  
- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，失败后不会有回调通知，详细描述见[错误码定义](#错误码定义)   

- 回调通知：

  ``` js
  youmeim.OnBlockUser = function(errorcode, userID, block) {}
  ```
    
- 参数：
  `errorcode `：错误码
  `userID `：用户ID
  `block `：true-屏蔽 false-解除屏蔽    

#### 解除所有已屏蔽用户 
- 接口示例：

  ``` js
      var errorcode = youmeim.UnBlockAllUser()
  ```
  
- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，失败后不会有回调通知，详细描述见[错误码定义](#错误码定义)  

- 回调通知：

  ``` js
  youmeim.OnUnBlockAllUser = function(errorcode) {}
  ```
    
- 回调参数：
  `errorcode `：错误码
  
#### 获取被屏蔽消息用户 
获取被自己屏蔽接收消息的所有用户

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_GetBlockUsers()
  ```
  
- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，失败后不会有回调通知，详细描述见[错误码定义](#错误码定义)  

- 回调通知：

  ``` js
  youmeim.OnGetBlockUsers = function(errorcode, userList) {}
  ```
    
- 回调参数：
  `errorcode`：错误码
  `userList`：屏蔽用户ID列表 (字符串数组)

### 消息功能设置
#### 切换获取消息模式 
在自动接收消息和手动接收消息间切换，默认是自动接收消息。

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_SetReceiveMessageSwitch(arrTargets， autoReceive)
  ```
  
- 参数：
  `arrTargets`：频道ID的数组，比如 `["room1","room2"]`
  `autoReceive`：`1`为自动接收消息，`0`为手动接收消息，默认为`1`。
  
- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，详细描述见[错误码定义](#错误码定义)  

#### 手动获取消息  
在手动接收消息模式，需要调用该接口后才能收到 `OnRecvMessage` 通知。

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_GetNewMessage( arrTargets )
  ```
  
- 参数：
  `arrTargets`：频道ID的数组，比如`["room1","room2"]`
  
- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，失败后不会有回调通知，详细描述见[错误码定义](#错误码定义)  

#### 新消息通知 
在手动接收消息模式，若设置了OnRecvNewMessage的监听，会通知新消息的数量。

- 通知接口：

  ``` js
      youmeim.OnRecvMessageNotify = function( chattype, targetID ) {}
  ```
  
- 回调参数：
  `chattype`：聊天类型，可以用于区分是频道聊天还是私聊
  `targetID`：如果是频道聊天，该值为频道ID；私聊该值为空字符串  
      
### 消息记录管理
#### 设置是否保存频道聊天记录 
设置是否在本地保存频道聊天记录，默认不保存。`私聊历史记录默认保存`。

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_SetRoomHistoryMessageSwitch( arrTargets, save )
  ```
  
- 参数：
  `arrTargets`：要设置的频道ID数组，比如 `["room1","room2"]`
  `save`：`1` 为保存，`0`不保存，默认'0'。
  
- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，详细描述见[错误码定义](#错误码定义)  

#### 拉取频道最近聊天记录 
从服务器拉取频道最近的聊天历史记录。
这个功能默认不开启，需要的请联系我们修改服务器配置。联系我们，可以通过专属游密支持群或者技术支持的大群。

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_QueryRoomHistoryMessageFromServer(roomID,count,direction)
  ```
  
- 参数：
  `roomID`：字符串,频道id
  `count`：整型，消息数量(最大200条)
  `directon`：历史消息排序方向 0：按时间戳升序	1：按时间戳逆序
  
- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，失败后不会有回调通知，详细描述见[错误码定义](#错误码定义) 

- 回调通知：

  ``` js
     // 与常规聊天一致
  youmeim.OnQueryRoomHistoryMessageFromServer = function(errorcode, roomID, remain, messageList){}
  ```
  
- 回调参数：
  `errorcode`：错误码
  `roomID`： 频道ID
  `remain`：剩余消息数量
  `messageList`：消息列表
  
- 备注：
  `messageList`中的每一条消息包含的关键字如下：    
  `ChatType`：聊天类型，私聊/频道聊天
  `MessageType`：消息类型，0-未知类型，1-文本消息，2-自定义消息，3-表情，4-图片，5-语音，6-视频，7-文件，8-礼物
  `Serial`：消息ID
  `CreateTime`：消息发送时间
  `ReceiveID`：消息接收者ID
  `SenderID`：消息发送者ID
  
  若MessageType=1（文本消息）有如下关键字：
  `Content`：文本消息内容
  
  若MessageType=5（语音消息）有如下关键字：
  `Text`：若使用的是语音转文字录音，此值为语音识别的文本内容，否则是空字符串
  `Param`：发送语音时的附加参数
  `Duration`：语音时长（单位：秒）
  
  若MessageType=8（礼物消息）有如下关键字：
  `Param`：附加参数
  `GiftID`：礼物ID
  `GiftCount`：礼物数量
  `Anchor`：主播ID 

#### 本地历史记录管理
##### 查询本地聊天历史记录 

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_QueryHistoryMessage(strTargetID, chatType, strStartMessageID , intCount, intDirection )
  ```
  
- 参数：
  `strTargetID`：私聊为用户的id／频道聊天为频道ID，字符串
  `chatType`： 聊天类型,私聊／频道聊天
  `strStartMessageID`：起始历史记录消息id（与requestid不同），为`"0"`表示首次查询，将倒序获取`count`条记录（lua不支持long，所以使用的是数字字符串）
  `intCount`：最多获取多少条
  `intDirection`：历史记录查询方向，`startMessageID=0`时,direction使用默认值0；`strStartMessageID>0`时，`0`表示查询比`strStartMessageID`小的消息，`1`表示查询比`strStartMessageID`大的消息
  
- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，失败后不会有回调通知，详细描述见[错误码定义](#错误码定义)  

- 相关函数：
  对于房间本地记录，需要先设置自动保存房间消息。`IM_SetRoomHistoryMessageSwitch`
    
- 回调通知：

  ``` js
  youmeim.OnQueryHistoryMsg = function(targetID,remainCount,msgLists){
      for(var i=0;i<msgLists.length;i++)
      {
        cc.log("OnQueryHistoryMsg： chattype：" + msgLists[i].ChatType + "Duration:" + msgLists[i].Duration + "|" + msgLists[i].MessageType + "|" +  msgLists[i].Param + "|" +  msgLists[i].ReceiveID + "|" +  msgLists[i].SenderID+  msgLists[i].Serial+  msgLists[i].Text + msgLists[i].LocalPath)
      }
 }
 ```
 
- 回调参数：
  `targetID`：用户ID/频道ID
  `remainCount`：剩余历史消息条数
  `msgLists`：消息列表
  
  `msgLists`中的每一条消息包含以下关键字：
  `MessageType`：消息类型，0-未知类型，1-文本消息，2-自定义消息，3-表情，4-图片，5-语音，6-视频，7-文件，8-礼物
  `ChatType`：聊天类型，私聊/频道聊天
  `ReceiveID`：消息接收者ID
  `SenderID`：消息发送者ID
  `Serial`：消息ID
  
  若MessageType=1（文本消息）有如下关键字：
  `Content`：文本消息内容
  
  若MessageType=5（语音消息）有如下关键字：   
  `Duration`：语音消息时长（单位：秒）
  `Param`：语音消息的自定义附加参数
  `Text`：语音消息的文本识别内容
  `LocalPath`：语音消息文件的本地路径

##### 根据时间清理本地聊天历史记录 
建议定期清理本地历史记录。

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_DeleteHistoryMessage( chatType , strTime)
  ```
  
- 参数：
  `chatType`：指定是清理频道消息还是私聊消息,聊天类型，1私聊，2频道聊天
  `strTime`：数字字符串，Unix timestamp,精确到秒，表示删除这个时间点之前的所有历史记录
  
- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，详细描述见[错误码定义](#错误码定义)  

##### 根据ID清理本地聊天历史记录 
- 接口示例：

  ``` js
      var errorcode = youmeim.IM_DeleteHistoryMessageByID( strMessageID )
  ```
  
- 参数：
  `strMessageID`：数字字符串，如果指定了大于"0"的值，将删除指定消息id的历史记录
  
- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，详细描述见[错误码定义](#错误码定义)  

##### 获取最近私聊联系人列表 
该接口是根据本地历史消息记录生成的最近联系人列表，按最后聊天时间倒序排列。该列表会受清理历史记录消息的接口影响。

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_GetContact()
  ```
  
- 参数：
  无。
  
- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，失败后不会有回调通知，详细描述见[错误码定义](#错误码定义) 

- 回调通知：

  ``` js
  youmeim.OnGetContactList = function(errorcode,contactLists)
  {
      for(var i=0;i<contactLists.length;i++)
      {
        cc.log("OnGetContactList " + contactLists[i])
      }
  }
  ```
  
- 回调参数：
  `errorcode`：错误码
  `contactLists`：最近联系人列表
  
  `contactLists`中每一条记录包含以下关键字：  
  `ContactID`：联系人ID
  `MessageType`：消息类型，0-未知类型，1-文本消息，2-自定义消息，3-表情，4-图片，5-语音，6-视频，7-文件，8-礼物
  `MessageContent`：消息内容
  `CreateTime`：消息创建时间  
    
### 高级功能    
#### 文本翻译 
将文本翻译成指定语言的文本，异步返回结果

- 接口示例：

  ``` js
      var requestID = youmeim.IM_TranslateText(strText, destLanguage, srcLanguage);
  ```

- 参数：
  `strText`：待翻译文本，字符串
  `destLanguage`：目标语言编码
  `srcLanguage`：原语言编码
 
- 返回值：  
`requestid`：消息序列号,字符串类型（实际是长整型数字），`-1`表示调用失败，失败后不会有回调通知。  

- 回调通知：

  ``` js
  youmeim.OnTranslateTextComplete = function(errorcode, requestID, text, srcLangCode, destLangCode) {}
  ```
  
- 回调参数：  
  `errorcode`：错误码，0表示成功
  `requestID`：请求ID
  `srcLanguage`：原语言编码
  `destLanguage`：目标语言编码   
    
#### 敏感词过滤 
考虑到客户端可能需要传递一些自定义消息，关键字过滤方法就直接提供出来，客户端可以选择是否过滤关键字，并且可以根据匹配到的等级能进行广告过滤。比如`level`返回的值为'2'表示广告，那么可以在发送时选择不发送，接收方可以选择不展示。

- 接口示例：

  ``` js
      var filter = youmeim.IM_GetFilterText(source_string)
    // filter.text
    // filter.level
  ```
  
- 参数：
  `source_string`：消息原文，字符串

- 返回值：
  `filter.text`:过滤敏感词后的消息，敏感词会替换为"*"。
  `filter.level`:包含敏感词的等级
    
## 游戏暂停与恢复
### 游戏暂停通知 
建议游戏放入后台时通知该接口，以便于得到更好重连效果。
调用OnPause(false),在游戏暂停后,若IM是登录状态，依旧接收IM消息；
调用OnPause(true),游戏暂停，即使IM是登录状态也不会接收IM消息；在游戏恢复运行时会主动拉取暂停期间未接收的消息，收到OnRecvMessage()回调。

- 接口示例：

  ``` js
      youmeim.IM_OnPause(pauseReceiveMessage)
  ```

- 参数：
  `pauseReceiveMessage`：是否暂停接收IM消息，true-暂停接收，false-不暂停接收

### 游戏恢复运行通知 
建议游戏从后台激活到前台时通知该接口，以便于得到更好重连效果。

- 接口示例：

  ``` js
      youmeim.IM_OnResume()
  ```
  
- 参数：
  无。   
  
## 用户举报和禁言
### 用户举报 
`IM_Accusation`接口提供举报功能，对用户违规的发言内容进行举报，管理员在后台进行审核处理并将结果通知用户。

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_Accusation(userID, source, reason, description, extraParam)
  ```
  
- 参数：
  `userID`：被举报用户ID,字符串
  `source`：来源（私聊 频道）,int
  `reason`：原因，由用户自定义枚举,int
  `description`：原因描述,字符串
  `extraParam`：附加信息JSON格式 ,字符串({"nickname":"","server_area":"","level":"","vip_level":""}）
  
- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，失败后不会有回调通知，详细描述见[错误码定义](#错误码定义)  

- 举报处理结果通知
当管理员对举报进行审核处理后，会将举办处理结果通知用户

- 原型

``` js
youmeim.OnAccusationResultNotify = function(result, userID, accusationTime) {}
```

- 回调参数：
  `result`：处理结果，0-忽略，1-警告，2-禁言
  `userID`：被举报用户ID
  `accusationTime`：举报时间 
  
### 禁言查询 
用户查询其所在频道的禁言状态。

- 接口示例：

  ``` js
      var errorcode = youmeim.GetForbiddenSpeakInfo() 
  ```

- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，失败后不会有回调通知，详细描述见[错误码定义](#错误码定义)    
    
- 回调通知：

  ``` js    
  youmeim.OnGetForbiddenSpeakInfo = function( errorcode , forbiddenInfos )
  ```
  
- 回调参数：
  `errorcode`：错误码
  `forbiddenInfos`：禁言状态列表,每一个代表一个频道的禁言状态
  
  `forbiddenInfos`中每条记录包含以下关键字：   
  `ChannelID`：频道ID
  `IsForbidRoom`：此频道是否被禁言，true-被禁言，false-未被禁言
  `reasonType`：禁言原因类型，0-未知，1-发广告，2-侮辱，3-政治敏感，4-恐怖主义，5-反动，6-色情，7-其它
  `forbidEndTime`：若被禁言，禁言结束时间     
      
## 公告管理
基本流程：申请开通公告功能->后台添加新公告然后设置公告发送时间,公告消息类型,发送时间,接收公告的频道等。->(客户端流程) 设置对应监听-> 调用对应接口->回调接收
### 公告功能简述

1.公告发送由后台配置，如类型、周期、发送时间、内容、链接、目标频道、次数、起始结束时间等。

2.公告三种类型：跑马灯，聊天框，置顶公告，
(1) 跑马灯，聊天框公告可设置发送时间，次数和间隔（从指定时间点开始隔固定间隔时间发送多次，界面展示及显示时长由客户端决定）;
(2) 置顶公告需设置开始和结束时间（该段时间内展示）。

3.三种公告均有一次性、周期性两种循环属性，
  一次性公告，到达指定时间点，发送该条公告；
  周期性公告，跟一次性公告发送规则一致，但是可以设置发送周期（在每周哪几天的指定时间发送）。
  
4.跑马灯与聊天框公告只有发送时间点在线的用户才能收到该公告，显示规则由客户端自己决定，两者区别主要是界面显示的区分。

5.置顶公告有显示起始和结束时间，表示该时段内显示，公告发送时间点在线的用户会收到该公告，公告发送时间点未在线用户，在公告显示时段登录，登录后可通过查询公告接口查到该公告。

6.公告撤销
仅针对置顶公告，公告显示时段撤销公告，客户端会收到公告撤销通知，界面进行更新。

### 接收公告
管理员在后台发布公告，当到达指定时间会收到该公告，界面根据不同类型的公告进行展示。

- 接收公告通知：

  ``` js
  youmeim.OnRecvNotice = function(noticeID, channelID, noticeType, content, linkText, linkAddress, beginTime, endTime) {}
  ```
    
- 参数：
  `noticeID`：公告Id
  `channelID`：发布公告的频道ID
  `noticeType`：公告类型
  `content`：公告内容
  `linkText`：公告链接关键字
  `linkAddress`：公告链接地址
  `beginTime`：公告开始时间
  `endTime`：公告结束时间

### 撤销公告
对于某些类型的公告（如置顶公告），需要在界面展示一段时间，如果管理员在该时间段执行撤销该公告，会收到撤销公告通知，界面进行相应更新。

- 撤销公告通知：

  ``` js
  youmeim.OnCancelNotice = function(noticeID, channelID) {}
  ```
  
- 参数：
  `noticeID`：公告ID
  `channelID`：发布公告的频道ID

### 查询公告 
公告在配置的时间点下发到客户端，对于某些类型的公告（如置顶公告）需要在某个时间段显示在，如果用户在公告下发时间点未在线，而在公告展示时间段内登录，应用可根据自己的需要决定是否展示该公告，`IM_QueryNotice`查询公告，结果通过上面的`OnRecvNotice`异步回调返回。

- 接口示例：

  ``` js
      var errrorcode = youmeim.IM_QueryNotice()
  ```
  
- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，失败后不会有回调通知，详细描述见[错误码定义](#错误码定义)            
      
## 地理位置
### 获取当前的地理位置 
- 接口示例：

  ``` js
      var errorcode = youmeim.IM_GetCurrentLocation()
  ```
  
- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，失败后不会有回调通知，详细描述见[错误码定义](#错误码定义)  

- 回调通知：

  ``` js
  youmeim.OnUpdateLocation = function(districtCode, country, pronvice, city, districtCounty, street, longitude, latitude)
  ```
  
- 回调参数：  
  `districtCode`：地理区域编码
  `country`：国家
  `pronvice`：省份
  `city`：城市
  `districtCounty`：区县
  `street`：街道
  `longitude`：经度
  `latitude`：纬度

### 获取附近的人
获取附近的目标(人 房间) ,若需要此功能，请联系我们开启LBS服务。若已开启服务，此功能生效的前提是自己和附近的人都获取了自己的地理位置，即调用了IM的获取当前地理位置接口。

- 条件：需要开通LBS定位服务才能使用

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_GetNearbyObjects(count, serverAreaID, districtLevel, resetStartDistance)
  ```

- 参数：
  `count`：获取附近的目标数量（一次最大200），int
  `serverAreaID`：区服ID（对应设置用户信息中的区服）,字符串
  `districtLevel`：行政区域等级，一般填0，0-未知，1-国家，2-省份，3-城市，4-区县，5-街道,int
  `resetStartDistance`：是否重置查找起始距离,1-从距自己0米开始查找，0-从上次查找返回的最远用户开始查找，int
  
- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，失败后不会有回调通知，详细描述见[错误码定义](#错误码定义)  

- 回调通知：

  ``` js
  youmeim.OnGetNearbyObjects = function(errorcode, neighbourList, startDistance, endDistance) {}
  ```
  
- 回调参数：  
  `errorcode`：错误码，0表示成功
  `neighbourList`：附近的人地理位置信息列表
  `startDistance`：搜索的起始距离
  `endDistance`：搜索的结束距离
  
  `neighbourList`中的每条记录包含以下关键字：  
  `UserID`：附近用户ID
  `Distance`：与自己的距离
  `Longitude`：经度
  `Latitude`：纬度
  `Country`：国家
  `Province`：省份
  `City`：城市
  `DistrictCounty`：区县
  `Street`：街道

### 地理位置更新 
从资源和耗电方面的考虑，SDK不自动监听地理位置的变化，如果调用方有需要可调用`IM_SetUpdateInterval`接口，设置更新时间间隔，SDK会按设定的时间间隔监听位置变化并通知上层。（如果应用对地理位置变化关注度不大，最好不要设置自动更新）

- 接口示例：

  ``` js
      youmeim.IM_SetUpdateInterval（interval）
  ```

- 参数：
  interval：更新时间间隔（单位：分钟）,int

### 获取与指定用户距离  
获取与指定用户距离之前，需要调用`IM_GetCurrentLocation`成功获取自己的地理位置,指定的用户也调用`IM_GetCurrentLocation`成功获取其地理位置。

- 接口示例：

  ``` js
      var errorCode = IM_GetDistance(userID);
  ```

- 参数：
  `userID`：用户ID，字符串
  
- 返回值：
  `errorcode`:`0`为调用成功，`非0`表示调用失败，失败后不会有回调通知，详细描述见[错误码定义](#错误码定义) 

- 回调通知：

  ``` js
  youmeim.OnGetDistance = function(errorcode, userID, distance) {}
  }  
  ```

- 回调参数：
  `errorcode`：错误码
  `userID`: 用户ID
  `distance`: 距离（米）

## 关系链管理
### 用户信息管理
#### 用户资料变更的通知
当好友的用户资料变更时会收到此通知，使用方根据需要决定是否重新获取资料变更好友的用户信息。

- 变更通知：

  ``` js
  youmeim.OnUserInfoChangeNotify = function(userID) {}
  ```

- 参数：
  `userID`：资料变更用户的用户ID 
  
#### 设置用户信息
设置用户的基本资料，昵称，性别，个性签名，地理位置等。

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_setUserProfileInfo(profileInfo)
  ```

- 参数：
  `profileInfo`：用户基本信息,json格式，所需字段如下：  

  `NickName`：昵称，长度最大为64bytes
  `Sex`：性别， 
         0-未知性别
         1-男性
         2-女性
  `Signature`：个性签名，长度最大为120bytes
  `Country`：国家  
  `Province`：省份
  `City`：城市
  `ExtraInfo`：扩展信息，，若需要此字段，传入JSON格式的内容， 不需要可传空字符串  
  
- 返回值：
  错误码，详细描述见[错误码定义](#错误码定义)   
    
- 回调通知：

  ``` js   
  youmeim.OnSetUserInfo = function(errorcode) {}
  ```
  
- 回调参数：
  `errorcode`：错误码
  
#### 查询用户基本资料
查询用户的基本资料，昵称，性别，个性签名，头像，地理位置，被添加权限等。

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_GetUserProfileInfo(userID)
  ```

- 参数：
  `userID`：用户ID，字符串
  
- 返回值：
  错误码，详细描述见[错误码定义](#错误码定义)    
    
- 回调通知：

  ``` js   
  youmeim.OnQueryUserInfo = function(errorcode,userID,photoUrl,onlineState,beAddPermission,foundPermission,nickName,sex,signature,country,province,city,extraInfo) {}
  ``` 
  
- 回调参数：
  `errorcode`：错误码
  `userID`：用户ID
  `photoUrl`：头像url
  `onlineState`：在线状态，
                 0-在线，默认值（已登录）
                 1-离线
                 2-隐身                
  `beAddPermission`：被好友添加权限
                   0-不允许被添加
                   1-需要验证
                   2-允许被添加，不需要验证, 默认值  
  `foundPermission`：能否被查找的权限
                    0-能被其它用户查找到，默认值
                    1-不能被其它用户查找到                    
  `nickName`：昵称，长度最大为64bytes
  `sex`：性别 
         0-未知性别
         1-男性
         2-女性
  `signature`：个性签名，长度最大为120bytes
  `country`：国家  
  `province`：省份
  `city`：城市
  `extraInfo`：扩展信息
  
#### 设置用户头像

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_SetUserProfilePhoto(photoPath)
  ```

- 参数：
  `photoPath`：本地图片绝对路径,url长度在500bytes内，图片大小在100kb内，字符串
  
- 返回值：
  错误码，详细描述见[错误码定义](#错误码定义)    
    
- 回调通知：

  ``` js    
  youmeim.OnSetPhotoUrl = function(errorcode,photoUrl) {}
  ```
  
- 回调参数：
  `errorcode`：错误码
  `photoUrl`：图片下载路径  
  
#### 切换用户在线状态

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_SwitchUserStatus(userID,userStatus)
  ```

- 参数：
  `userID`：用户ID，字符串
  `userStatus`：用户在线状态，
                0-在线，默认值（已登录）
                1-离线
                2-隐身
  
- 返回值：
  错误码，详细描述见[错误码定义](#错误码定义)    
    
- 回调通知：

  ``` js   
  youmeim.OnSwitchUserOnlineState = function(errorcode) {}
  ```
  
- 回调参数：
  `errorcode`：错误码 
  
#### 设置好友添加权限

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_SetAddPermission(beFound,beAddPermission)
  ```

- 参数：
  `beFound`：能否被别人查找到，true-能被查找，false-不能被查找
  `beAddPermission`：被其它用户添加的权限， 
                     0-不允许被添加
                     1-需要验证
                     2-允许被添加，不需要验证, 默认值  
  
- 返回值：
  错误码，详细描述见[错误码定义](#错误码定义)    
    
- 回调通知：

  ``` js    
  youmeim.OnSetUserInfo = function(errorcode) {}
  ```
  
- 回调参数：
  `errorcode`：错误码  
  
### 好友管理  
#### 查找好友
查找将要添加的好友，获取该好友的简要信息。

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_FindUser(findType,target)
  ```

- 参数：
  `findType`：查找类型，0-按ID查找，1-按昵称查找
  `target`：对应查找类型选择的用户ID或昵称，字符串
  
- 返回值：
  错误码，详细描述见[错误码定义](#错误码定义)    
    
- 回调通知：

  ``` js   
  youmeim.OnFindUser = function(errorcode,userList) {}
  ```
  
- 回调参数：
  `errorcode`：错误码 
  `userList`：用户简要信息列表
  
- 备注：
  `userList` 中每条记录的关键字如下：  
  `UserID`：用户ID
  `Nickname`：用户昵称
  `Status`：在线状态
            0-在线
            1-离线
            2-隐身     
       
#### 添加好友

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_RequestAddFriend(users,comments)
  ```

- 参数：
  `users`：需要添加为好友的用户ID列表,数组类型
  `comments`：备注或验证信息，长度最大128bytes，字符串
  
- 返回值：
  错误码，详细描述见[错误码定义](#错误码定义)    
    
- 回调通知：

  ``` js   
  youmeim.OnRequestAddFriend = function(errorcode,userID) {}
  ```
  
- 回调参数：
  `errorcode`：错误码 
  `userID`：用户ID
  
- 备注：
  被请求方会收到被添加为好友的通知，被请求方如果设置了需要验证才能被添加，会收到下面的回调：
  
  ``` js   
  youmeim.OnBeRequestAddFriendNotify = function(userID,comments) {}
  ```   
  
  - 回调参数：     
    `userID`：请求方的用户ID
    `comments`：备注或验证信息
    
  被请求方如果设置了不需要验证就能被添加，收到下面的回调：
  
  ``` js  
  youmeim.OnBeAddFriendNotify = function(userID,comments) {}
  ```   
  
  - 回调参数：     
    `userID`：请求方的用户ID
    `comments`：备注或验证信息  
  
#### 处理好友请求
当前用户有被其它用户请求添加为好友的请求时，处理添加请求。

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_DealBeRequestAddFriend(userID,dealResult);
  ```

- 参数：
  `userID`：请求方的用户ID，字符串
  `dealResult`：处理结果，0-同意，1-拒绝
  
- 返回值：
  错误码，详细描述见[错误码定义](#错误码定义)    
    
- 回调通知：

  ``` js 
 youmeim.OnDealBeRequestAddFriend = function(errorcode,userID,comments,dealResult) {}
  ```
  
- 回调参数：
  `errorcode`：错误码 
  `userID`：请求方的用户ID
  `comments`：备注或验证信息
  `dealResult`：处理结果，0-同意，1-拒绝 
  
- 备注：
 
   如果被请求方设置了需要验证才能被添加为好友，在被请求方成功处理了请求方的好友请求后，请求方能收到添加好友请求结果的通知，收到下面的回调：
   
  ``` js  
  youmeim.OnRequestAddFriendResultNotify = function(userID,comments,dealResult) {}
  ```
  
- 回调参数：   
  `userID`：被请求方的用户ID
  `comments`：备注或验证信息
  `dealResult`：处理结果，0-同意，1-拒绝  
    
#### 删除好友

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_DeleteFriend(users, deleteType)
  ```

- 参数：
  `users`：需删除的好友用户ID列表,数组类型
  `deleteType`：删除类型，0-双向删除，1-单向删除(删除方在被删除方好友列表依然存在)
  
- 返回值：
  错误码，详细描述见[错误码定义](#错误码定义)    
    
- 回调通知：

  ``` js  
  youmeim.OnDeleteFriend = function(errorcode,userID) {}
  ```
  
- 回调参数：
  `errorcode`：错误码 
  `userID`：被删除好友的用户ID
  
- 备注：
 
   如果删除方采用的是双向删除，被删除方能收到其被好友删除的通知，收到下面的回调：
   
  ``` js   
  youmeim.OnBeDeleteFriendNotify = function(userID) {}
  ```
  
- 回调参数：   
  `userID`：删除方的用户ID   
  
#### 拉黑好友

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_BlackFriend(type, users)
  ```

- 参数：
  `type`：拉黑类型，0-拉黑，1-解除拉黑
  `users`：需拉黑的好友用户ID列表,数组类型  
  
- 返回值：
  错误码，详细描述见[错误码定义](#错误码定义)    
    
- 回调通知：

  ``` js   
  youmeim.OnBlackFriend = function(errorcode,type,userID) {}
  ```
  
- 回调参数：
  `errorcode`：错误码 
  `type`：拉黑类型，0-拉黑，1-解除拉黑
  `userID`：被拉黑方用户ID    
  
#### 查询好友列表
查询当前用户已添加的好友，也能查找被拉黑的好友。

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_QueryFriends(type, startIndex, count)
  ```

- 参数：
  `type`：查找类型，0-正常状态的好友，1-被拉黑的好友
  `startIndex`：起始序号，作用为分批查询好友（例如：第一次从序号0开始查询30条，第二次就从序号30开始查询相应的count数）
  `count`：数量（一次最大100）  
  
- 返回值：
  错误码，详细描述见[错误码定义](#错误码定义)    
    
- 回调通知：

  ``` js 
  youmeim.OnQueryFriends = function(errorcode,type,startIndex,firendList) {}
  ```
  
- 回调参数：
  `errorcode`：错误码 
  `type`：查找类型，0-正常状态的好友，1-被拉黑的好友
  `startIndex`：起始序号
  `firendList`：好友列表 
  
- 备注：
  `firendList` 中每条记录的关键字如下：  
  `UserID`：用户ID
  `Nickname`：用户昵称
  `Status`：在线状态
            0-在线
            1-离线
            2-隐身   
  
#### 查询好友请求列表
查询当前用户收到的被添加请求的好友列表。

- 接口示例：

  ``` js
      var errorcode = youmeim.IM_QueryFriendRequestList(startIndex, count)
  ```

- 参数：  
  `startIndex`：起始序号，作用为分批查询好友请求（例如：第一次从序号0开始查询10条，第二次就从序号10开始查询相应的count数）
  `count`：数量（一次最大20）  
  
- 返回值：
  错误码，详细描述见[错误码定义](#错误码定义)    
    
- 回调通知：

  ``` js   
  youmeim.OnQueryFriendRequestList = function(errorcode,startIndex,requestList) {}
  ```
  
- 回调参数：
  `errorcode`：错误码 
  `startIndex`：起始序号
  `requestList`：好友请求列表
  
- 备注：
  `requestList` 中每条记录的关键字如下：  
  `AskerID`：请求方ID
  `AskerNickname`：请求方昵称
  `InviteeID`：受邀方ID
  `InviteeNickname`：受邀方昵称
  `ValidateInfo`：验证信息
  `Status`：好友请求状态
            0-添加完成
            1-添加失败  
            2-等待对方验证 
            3-等待我验证
  `CreateTime`：请求的发送或接收时间   

## 全球布点选择
**必须在初始化之前调用** 根据游戏主要发行区域，选择合适的 IM 服务器区域。

- 接口示例：

  ``` js
      youmeim.IM_SetServerZone(zone)
  ```
  
- 参数：
  `zone`：整型，见[服务器部署地区定义](#服务器部署地区定义)

## 服务器部署地区定义

``` js
      ServerZone_China = 0,   // 中国大陆，默认
      ServerZone_Singapore = 1, // 新加坡
      ServerZone_America = 2,   // 美国
      ServerZone_HongKong = 3,  // 香港
      ServerZone_Korea = 4,   // 韩国
      ServerZone_Australia = 5, // 澳洲
      ServerZone_Deutschland = 6, // 德国
      ServerZone_Brazil = 7,    // 巴西
      ServerZone_India = 8,   // 印度
      ServerZone_Japan = 9,   // 日本
      ServerZone_Ireland = 10,  // 爱尔兰
```
    
## 错误码定义

| 错误码                                               | 含义         |
| -------------                                       | -------------|
| YIMErrorcode_Success = 0                        | 成功|
| YIMErrorcode_EngineNotInit = 1                  | IM SDK未初始化|
| YIMErrorcode_NotLogin = 2                       | IM SDK未登录|
| YIMErrorcode_ParamInvalid = 3                   | 无效的参数|
| YIMErrorcode_TimeOut = 4                        | 超时|
| YIMErrorcode_StatusError = 5                    | 状态错误|
| YIMErrorcode_SDKInvalid = 6                     | Appkey无效|
| YIMErrorcode_AlreadyLogin = 7                   | 已经登录|
| YIMErrorcode_LoginInvalid = 1001                | 登录无效|
| YIMErrorcode_ServerError = 8                    | 服务器错误|
| YIMErrorcode_NetError = 9                       | 网络错误|
| YIMErrorcode_LoginSessionError = 10             | 登录状态出错|
| YIMErrorcode_NotStartUp = 11                    | SDK未启动|
| YIMErrorcode_FileNotExist = 12                  | 文件不存在|
| YIMErrorcode_SendFileError = 13                 | 文件发送出错|
| YIMErrorcode_UploadFailed = 14                  | 文件上传失败，上传失败 一般都是网络限制上传了| 
| YIMErrorcode_UsernamePasswordError = 15,         | 用户名密码错误|
| YIMErrorcode_UserStatusError = 16,               | 用户状态为无效用户|
| YIMErrorcode_MessageTooLong = 17,                | 消息太长|
| YIMErrorcode_ReceiverTooLong = 18,               | 接收方ID过长（检查频道名）|
| YIMErrorcode_InvalidChatType = 19,               | 无效聊天类型|
| YIMErrorcode_InvalidReceiver = 20,               | 无效用户ID|
| YIMErrorcode_UnknowError = 21,                   | 未知错误|
| YIMErrorcode_InvalidAppkey = 22,                 | AppKey无效|
| YIMErrorcode_ForbiddenSpeak = 23,                | 被禁止发言|
| YIMErrorcode_CreateFileFailed = 24,              | 创建文件失败|
| YIMErrorcode_UnsupportFormat = 25,               | 支持的文件格式|
| YIMErrorcode_ReceiverEmpty = 26,                 | 接收方为空|
| YIMErrorcode_RoomIDTooLong = 27,                 | 房间名太长|
| YIMErrorcode_ContentInvalid = 28,                | 聊天内容严重非法|
| YIMErrorcode_NoLocationAuthrize = 29,            | 未打开定位权限|
| YIMErrorcode_UnknowLocation = 30,                | 未知位置|
| YIMErrorcode_Unsupport = 31,                     | 不支持该接口|
| YIMErrorcode_NoAudioDevice = 32,                 | 无音频设备|
| YIMErrorcode_AudioDriver = 33,                   | 音频驱动问题|
| YIMErrorcode_DeviceStatusInvalid = 34,           | 设备状态错误|
| YIMErrorcode_ResolveFileError = 35,              | 文件解析错误|
| YIMErrorcode_ReadWriteFileError = 36,            | 文件读写错误|
| YIMErrorcode_NoLangCode = 37,                    | 语言编码错误|
| YIMErrorcode_TranslateUnable = 38,               | 翻译接口不可用|
| YIMErrorcode_SpeechAccentInvalid = 39,           | 语音识别方言无效|
| YIMErrorcode_SpeechLanguageInvalid = 40,         | 语音识别语言无效|
| YIMErrorcode_HasIllegalText = 41,                | 消息含非法字符|
| YIMErrorcode_AdvertisementMessage = 42,          | 消息涉嫌广告|
| YIMErrorcode_AlreadyBlock = 43,                  | 用户已经被屏蔽|
| YIMErrorcode_NotBlock = 44,                      | 用户未被屏蔽|
| YIMErrorcode_MessageBlocked = 45,                | 消息被屏蔽|
| YIMErrorcode_LocationTimeout = 46,               | 定位超时|
| YIMErrorcode_NotJoinRoom = 47,                   | 未加入该房间|
| YIMErrorcode_LoginTokenInvalid = 48,             | 登录token错误|
| YIMErrorcode_CreateDirectoryFailed = 49,         | 创建目录失败|
| YIMErrorcode_InitFailed = 50,                    | 初始化失败|
| YIMErrorcode_Disconnect = 51,                    | 与服务器断开|    
| YIMErrorcode_TheSameParam = 52,                  | 设置参数相同|
| YIMErrorcode_QueryUserInfoFail = 53,             | 查询用户信息失败|
| YIMErrorcode_SetUserInfoFail = 54,               | 设置用户信息失败|
| YIMErrorcode_UpdateUserOnlineStateFail = 55,     | 更新用户在线状态失败|
| YIMErrorcode_NickNameTooLong = 56,               | 昵称太长(> 64 bytes)|
| YIMErrorcode_SignatureTooLong = 57,              | 个性签名太长(> 120 bytes)|
| YIMErrorcode_NeedFriendVerify = 58,              | 需要好友验证信息|
| YIMErrorcode_BeRefuse = 59,                      | 添加好友被拒绝|
| YIMErrorcode_HasNotRegisterUserInfo = 60,        | 未注册用户信息|
| YIMErrorcode_AlreadyFriend = 61,                 | 已经是好友|
| YIMErrorcode_NotFriend = 62,                     | 非好友|
| YIMErrorcode_NotBlack = 63,                      | 不在黑名单中|
| YIMErrorcode_PhotoUrlTooLong = 64,               | 头像url过长(>500 bytes)|
| YIMErrorcode_PhotoSizeTooLarge = 65,             | 头像太大（>100 kb）|
| YIMErrorcode_ChannelMemberOverflow = 66,         | 达到频道人数上限 |
| YIMErrorcode_PTT_Start = 2000,                   | 开始录音|
| YIMErrorcode_PTT_Fail = 2001,                    | 录音失败|
| YIMErrorcode_PTT_DownloadFail = 2002,            | 语音消息文件下载失败|
| YIMErrorcode_PTT_GetUploadTokenFail = 2003,      | 获取语音消息Token失败|
| YIMErrorcode_PTT_UploadFail = 2004,              | 语音消息文件上传失败|
| YIMErrorcode_PTT_NotSpeech = 2005,               | 没有录音内容|
| YIMErrorcode_PTT_DeviceStatusError = 2006,        | 语音设备状态错误
| YIMErrorcode_PTT_IsSpeeching = 2007,              | 录音中|
| YIMErrorcode_PTT_FileNotExist = 2008,             | 文件不存在|
| YIMErrorcode_PTT_ReachMaxDuration = 2009,         | 达到最大时长限制|
| YIMErrorcode_PTT_SpeechTooShort = 2010,           | 录音时间太短|
| YIMErrorcode_PTT_StartAudioRecordFailed = 2011,   | 启动录音失败|
| YIMErrorcode_PTT_SpeechTimeout = 2012,            | 音频输入超时|
| YIMErrorcode_PTT_IsPlaying = 2013,                | 在播放|
| YIMErrorcode_PTT_NotStartPlay = 2014,             | 未开始播放|
| YIMErrorcode_PTT_CancelPlay = 2015,               | 主动取消播放|
| YIMErrorcode_PTT_NotStartRecord = 2016,           | 未开始语音|
| YIMErrorcode_PTT_NotInit = 2017,                  | 未初始化|
| YIMErrorcode_PTT_InitFailed = 2018,               | 初始化失败|
| YIMErrorcode_PTT_Authorize = 2019,                | 录音权限|
| YIMErrorcode_PTT_StartRecordFailed = 2020,        | 启动录音失败|
| YIMErrorcode_PTT_StopRecordFailed = 2021,         | 停止录音失败|
| YIMErrorcode_PTT_UnsupprtFormat = 2022,           | 不支持的格式|
| YIMErrorcode_PTT_ResolveFileError = 2023,         | 解析文件错误|
| YIMErrorcode_PTT_ReadWriteFileError = 2024,       | 读写文件错误|
| YIMErrorcode_PTT_ConvertFileFailed = 2025,        | 文件转换失败|
| YIMErrorcode_PTT_NoAudioDevice = 2026,            | 无音频设备|
| YIMErrorcode_PTT_NoDriver = 2027,                 | 驱动问题|
| YIMErrorcode_PTT_StartPlayFailed = 2028,          | 启动播放失败|
| YIMErrorcode_PTT_StopPlayFailed = 2029,           | 停止播放失败|
| YIMErrorcode_PTT_RecognizeFailed = 2030,          | 识别失败|
| YIMErrorcode_Fail = 10000                         | 语音服务启动失败|

## 其他值定义
### 禁言原因：ForbidSpeakReason
| 错误码                       | 含义         |
| -------------                                       | -------------|
|0                            | 未知|
|1                            | 发广告|
|2                            | 侮辱|
|3                            | 政治敏感|
|4                            | 恐怖主义|
|5                            | 反动|
|6                            | 色情|
|7                            | 其他|


### 备注：
实际Demo点击此处下载->[Youme IM Demo for Cocos2d Js](http://dl2.youme.im/release/cocosCreateIMDemo.zip)


