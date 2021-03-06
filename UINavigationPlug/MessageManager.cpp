﻿#include "MessageManager.h"
#include "../Message/ChatMessage.h"
#include "../EventBus/EventBus.h"
#include "../Message/LoginMessgae.h"
#include <QDebug>
#include "../Message/StatusMessage.h"
#include "../Message/UserMessage.h"
#include "../QtUtil/Utils/Log.h"
#include "../Platform/Platform.h"

/**
  * @函数名
  * @功能描述
  * @参数
  * @date 2018.9.29
  */
void NavigationMsgManager::sendDownLoadHeadPhotosMsg(const std::vector<std::string> &_withoutHeadPhotos,
                                                     const std::set<std::string> &_withoutGroupHeadPhotos) {
    DownLoadHeadPhotoEvent event;
    event._withoutHeadPhotos = _withoutHeadPhotos;
    EventBus::FireEvent(event);

    DownLoadGroupHeadPhotoEvent eventg;
    eventg._withoutHeadPhotos.resize(_withoutGroupHeadPhotos.size());
    std::copy(_withoutGroupHeadPhotos.begin(), _withoutGroupHeadPhotos.end(), eventg._withoutHeadPhotos.begin());
    EventBus::FireEvent(eventg);
}

/**
  * @函数名
  * @功能描述
  * @参数
  * @date 2018.10.12
  */
void NavigationMsgManager::sendGetUserStatus(const std::set<std::string> &users) {
    GetUsersOnlineEvent e;
    e._users = users;
    EventBus::FireEvent(e);
}

/**
  * @函数名   retryConnecToServer
  * @功能描述 触发重连服务器
  * @参数
  * @author   cc
  * @date     2018/10/24
  */
//void NavigationMsgManager::retryConnecToServer() {
//    RetryConnectToServerEvt e;
//    EventBus::FireEvent(e);
//}

//
void NavigationMsgManager::sendReadedMessage(const std::string &messageId, const std::string& userId, QUInt8 chatType) {
    ReadedMessage e(messageId, userId, chatType);
    EventBus::FireEvent(e);
}

void NavigationMsgManager::getSessionData() {

    GetSessionData e;
    EventBus::FireEvent(e);
}

void NavigationMsgManager::removeSession(std::string &peerId) {
    RemoveSessionData e(peerId);
    EventBus::FireEvent(e);
}

void NavigationMsgManager::setUserSetting(bool isSetting, const std::string &key, const std::string &subKey,
                                          const std::string &val) {

    debug_log("setting userconfig -> issetting:{0}, key:{1}, subKey:{2}, val:{3}", isSetting, key, subKey, val);

    UserSettingMsg e;
    e.operatorType = isSetting ? UserSettingMsg::EM_OPERATOR_CANCEL : UserSettingMsg::EM_OPERATOR_SET;
    e.key = key;
    e.subKey = subKey;
    e.value = val;
    EventBus::FireEvent(e);
}

void NavigationMsgManager::addEmptyMessage(const QTalk::Entity::ImMessageInfo &info)
{
    //
    EmptyMessageEvt evt(info);
    EventBus::FireEvent(evt);
}

//
void NavigationMsgManager::quitGroupById(const std::string& groupId) {
    QuitGroupMsg e;
    e.groupId = groupId;
    EventBus::FireEvent(e);
}

/**
  * @函数名 MessageListener
  * @功能描述 构造监听器 并注册到eventbus中
  * @参数
  * @date 2018.9.20
  */
NavigationMsgListener::NavigationMsgListener(NavigationMainPanel *navigationMainPanel) :
        _pNavigationMainPanel(navigationMainPanel) {
	EventBus::AddHandler<R_Message>(*this);
	EventBus::AddHandler<S_Message>(*this);
	EventBus::AddHandler<DownloadHeadSuccess>(*this);
	EventBus::AddHandler<DownloadGroupHeadSuccess>(*this);
	EventBus::AddHandler<UpdateOnlineEvent>(*this);
	EventBus::AddHandler<GetUsersOnlineSucessEvent>(*this);
	EventBus::AddHandler<DisconnectToServer>(*this);
//	EventBus::AddHandler<RetryConnectRet>(*this);
	EventBus::AddHandler<SynOfflineSuccees>(*this);
	EventBus::AddHandler<UpdateGroupInfoRet>(*this);
	EventBus::AddHandler<SignalReadState>(*this);
	EventBus::AddHandler<GroupReadState>(*this);
	EventBus::AddHandler<RevokeMessage>(*this);
	EventBus::AddHandler<S_RevokeMessage>(*this);
	EventBus::AddHandler<UpdateUserConfigMsg>(*this);
	EventBus::AddHandler<DestroyGroupRet>(*this);
	EventBus::AddHandler<ChangeHeadRetMessage>(*this);
	EventBus::AddHandler<UserCardMessgae>(*this);
	EventBus::AddHandler<IncrementConfig>(*this);
	EventBus::AddHandler<RecvVideoMessage>(*this);
	EventBus::AddHandler<MStateEvt>(*this);
}

NavigationMsgListener::~NavigationMsgListener() {

}

/**
  * @函数名 onEvent
  * @功能描述 接受eventbus分发的消息
  * @参数
  * @date 2018.9.20
  */
void NavigationMsgListener::onEvent(R_Message &e) {
    _pNavigationMainPanel->receiveSession(e);
}

/**
  * @函数名 onEvent
  * @功能描述 接受eventbus分发的消息
  * @参数
  * @date 2018.10.26
  */
void NavigationMsgListener::onEvent(S_Message &e) {
    _pNavigationMainPanel->sendSession(e);
}


/**
  * @函数名
  * @功能描述 头像现在完刷新头像
  * @参数
  * @date 2018.9.29
  */
void NavigationMsgListener::onEvent(DownloadHeadSuccess &e) {

    if (_pNavigationMainPanel) {
        _pNavigationMainPanel->onDownLoadHeadPhotosFinish();
    }
}


/**
  * @函数名
  * @功能描述
  * @参数
  * @date 2018.9.30
  */
void NavigationMsgListener::onEvent(DownloadGroupHeadSuccess &e) {
    if (_pNavigationMainPanel) {
        _pNavigationMainPanel->onDownLoadGroupHeadPhotosFinish();
    }
}

/**
  * @函数名
  * @功能描述
  * @参数
  * @date 2018.10.12
  */
void NavigationMsgListener::onEvent(UpdateOnlineEvent &e) {
    if (_pNavigationMainPanel) {
        _pNavigationMainPanel->onUpdateOnline();
    }
}

/**
  * @函数名
  * @功能描述
  * @参数
  * @date 2018.10.15
  */
void NavigationMsgListener::onEvent(GetUsersOnlineSucessEvent &e) {
    if (_pNavigationMainPanel) {
        _pNavigationMainPanel->onUpdateOnlineUsers(e._userstatus);
    }
}

/**
  * @函数名   
  * @功能描述 
  * @参数     DisconnectToServer 断链消息
     void
  * @author   cc
  * @date     2018/10/23
  */
void NavigationMsgListener::onEvent(DisconnectToServer &e) {
    if (e.getCanceled()) return;

    if (_pNavigationMainPanel) {
        _pNavigationMainPanel->onTcpDisconnect();
    }
}

/**
  * @函数名   
  * @功能描述 
  * @参数
  * @author   cc
  * @date     2018/10/24
  */
//void NavigationMsgListener::onEvent(RetryConnectRet &e) {
//    if (e.getCanceled()) return;
//
//    if (_pNavigationMianPanel) {
//        _pNavigationMianPanel->onRetryConnected();
//    }
//}

/**
  * @函数名   
  * @功能描述 
  * @参数
  * @author   cc
  * @date     2018/10/24
  */
void NavigationMsgListener::onEvent(SynOfflineSuccees &e) {
    if (e.getCanceled()) {
        return;
    }
    if (nullptr != _pNavigationMainPanel) {
        _pNavigationMainPanel->onLoginSuccess();
    }
}

void NavigationMsgListener::onEvent(UpdateGroupInfoRet &e) {
    if (e.getCanceled()) {
        return;
    }
    if (nullptr != _pNavigationMainPanel) {
        _pNavigationMainPanel->onUpdateGroupInfo(e.groupinfo);
    }
}

void NavigationMsgListener::onEvent(SignalReadState &e) {

    if (e.getCanceled()) return;

    if (nullptr != _pNavigationMainPanel) {
        _pNavigationMainPanel->updateReadCount(QTalk::Entity::UID(e.userId, e.realJid), e.mapReadState.size());
    }
}

void NavigationMsgListener::onEvent(GroupReadState &e) {

    if (e.getCanceled()) return;

    if (nullptr != _pNavigationMainPanel) {
        auto it = e.mapReadCount.begin();
        for (; it != e.mapReadCount.end(); it++) {
            _pNavigationMainPanel->updateReadCount(QTalk::Entity::UID(it->first, it->first), it->second);
        }
    }
}

void NavigationMsgListener::onEvent(RevokeMessage &e) {

    if (e.getCanceled()) return;

    if (nullptr != _pNavigationMainPanel) {
        _pNavigationMainPanel->recvRevokeMessage(e.uid, e.messageFrom);
    }
}

void NavigationMsgListener::onEvent(S_RevokeMessage &e) {
    if (e.getCanceled()) return;

    if (nullptr != _pNavigationMainPanel) {
        _pNavigationMainPanel->recvRevokeMessage(e.uid, e.messageFrom);
    }
}

void NavigationMsgListener::onEvent(UpdateUserConfigMsg &e) {

    if (e.getCanceled()) return;

    if (nullptr != _pNavigationMainPanel) {
        _pNavigationMainPanel->onUpdateUserConfig(e.arConfigs);
    }
}

void NavigationMsgListener::onEvent(DestroyGroupRet &e) {
    if (e.getCanceled()) return;

    if (_pNavigationMainPanel) {
        _pNavigationMainPanel->onDestroyGroup(e.groupId);
    }
}

/**
 *
 * @param e
 */
void NavigationMsgListener::onEvent(ChangeHeadRetMessage &e)
{
    if(e.getCanceled()) return;

    if(_pNavigationMainPanel)
    {
        std::string xmppId = PLAT.getSelfXmppId();
        _pNavigationMainPanel->onChangeHeadRet(e.ret, xmppId, e.localHead);
    }
}

void NavigationMsgListener::onEvent(UserCardMessgae &e)
{
    if(e.getCanceled()) return;

    if(_pNavigationMainPanel)
    {
        auto userCards = e.userCards;
        for(const auto& userCard : userCards)
        {
            std::string localHead = QTalk::GetHeadPathByUrl(userCard.headerSrc);
            _pNavigationMainPanel->onChangeHeadRet(true, userCard.xmppId, localHead);
        }
    }
}

void NavigationMsgListener::onEvent(IncrementConfig &e) {
    if(e.getCanceled()) return;

    if (nullptr != _pNavigationMainPanel) {
        _pNavigationMainPanel->onUpdateUserConfig(e.deleteData, e.arImConfig);
    }
}

void NavigationMsgListener::onEvent(RecvVideoMessage &e) {
    if (nullptr != _pNavigationMainPanel) {

    }
}

void NavigationMsgListener::onEvent(MStateEvt &e) {
    if (nullptr != _pNavigationMainPanel) {
        _pNavigationMainPanel->onGotMState(QTalk::Entity::UID(e.userId, e.realJid), e.messageId.data(), e.time);
    }
}
