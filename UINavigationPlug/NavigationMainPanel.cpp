﻿#include "NavigationMainPanel.h"
#include "SessionFrm.h"
#include <QDebug>
#include <QTimer>
#include <QtConcurrent>
#include "../UICom/UIEntity.h"
#include "MessageManager.h"
#include <QMetaType>
#include <string>
#include <QLabel>
#include <QFont>
#include <QTcpSocket>
#include <QNetworkConfigurationManager>
#include <QNetworkSession>
#include "TcpDisconnect.h"
#include "../Platform/Platform.h"
#include "../Platform/NavigationManager.h"
#include "../Platform/dbPlatForm.h"
#include "../QtUtil/Utils/Log.h"
#include "../QtUtil/Entity/JID.h"

NavigationMainPanel::NavigationMainPanel(QWidget *parent) :
        QFrame(parent),
        _mainLayout(nullptr),
        _stackWdt(nullptr),
        _pSessionFrm(nullptr),
        _pTcpDisconnect(nullptr)
//        _pConnToServerTimer(nullptr)
        {

    qRegisterMetaType<ReceiveSession>("ReceiveSession");
    qRegisterMetaType<QTalk::Entity::UID>("QTalk::Entity::UID");
    init();
}

NavigationMainPanel::~NavigationMainPanel() {
    if(_messageListener)
        delete _messageListener;
}

void NavigationMainPanel::receiveSession(R_Message mess)
{
	QTalk::Entity::ImMessageInfo message = mess.message;
	ReceiveSession info;
	info.chatType = (QTalk::Enum::ChatType)message.ChatType;
	info.messageId = QString::fromStdString(message.MsgId);
	info.messageContent = QString::fromStdString(message.Content);
	info.xmppId = QString::fromStdString(QTalk::Entity::JID(message.SendJid).basename());
	info.realJid = QString::fromStdString(QTalk::Entity::JID(message.RealJid).basename());
	info.messageRecvTime = message.LastUpdateTime;
	std::string from = QTalk::Entity::JID(message.From).basename();
	info.sendJid = QString::fromStdString(from);
	info.messtype = message.Type;


	emit sgReceiveSession(info, from == PLAT.getSelfXmppId());
	//qlog_info(QString("emit sessioon id:%1").arg(info.messageId));
}

void NavigationMainPanel::sendSession(S_Message mess) {
    QTalk::Entity::ImMessageInfo message = mess.message;
    ReceiveSession info;
    info.chatType = (QTalk::Enum::ChatType) message.ChatType;
    info.messageId = QString::fromStdString(message.MsgId);
    info.messageContent = QString::fromStdString(message.Content);
    info.xmppId = QString::fromStdString(message.To);
    info.realJid = QString::fromStdString(message.RealJid);
    info.messageRecvTime = message.LastUpdateTime;
    info.messtype = message.Type;

    info.sendJid = QString::fromStdString(PLAT.getSelfXmppId());
    emit sgReceiveSession(info, true);
}

/**
  * @功能描述
  * @参数
  * @author cc
  * @date 2018.9.17
  */
void NavigationMainPanel::init() {
    this->setFocusPolicy(Qt::NoFocus);
    initLayout();
    initMessage();
    connects();
//    if (nullptr == _pConnToServerTimer) {
//        _pConnToServerTimer = new QTimer(this);
//        _pConnToServerTimer->setInterval(5000);
//        connect(_pConnToServerTimer, &QTimer::timeout, this, &NavigationMianPanel::retryToConnect);
//    }
//        connect(this, SIGNAL(connToServerTimerSignal(bool)), this, SLOT(connToServerTimerSlot(bool)));
    //
    QtConcurrent::run([](){
        //
        NavigationMsgManager::getSessionData();
    });
//
//
//    //Set Internet Access Point
//    QNetworkConfigurationManager manager;
//    //Is there default access point, use it
//    QNetworkConfiguration cfg = manager.defaultConfiguration();
//
//    //Open session
//    auto *_session = new QNetworkSession(cfg);
//    connect(_session, &QNetworkSession::closed, [](){
//        qDebug() << "---------------closed";
//    });
//    connect(_session, &QNetworkSession::stateChanged, [](QNetworkSession::State state){
//        qDebug() << "----------------" << state;
//    });
//    _session->open();
//    _session->waitForOpened();
}

/**
  * @功能描述
  * @参数
  * @author cc
  * @date 2018.9.18
  */
void NavigationMainPanel::initLayout() {
    this->setObjectName("navigationMainPanel");
    if (nullptr == _mainLayout) {
        _mainLayout = new QVBoxLayout(this);
        _mainLayout->setSpacing(0);
        _mainLayout->setMargin(0);
    }
    if (nullptr == _pTcpDisconnect) {
        _pTcpDisconnect = new TcpDisconnect(this);
        _mainLayout->addWidget(_pTcpDisconnect);
        _pTcpDisconnect->setVisible(false);
    }
    if (nullptr == _stackWdt) {
        _stackWdt = new QStackedWidget(this);
        _mainLayout->addWidget(_stackWdt);
    }
    if (nullptr == _pSessionFrm) {
        _pSessionFrm = new SessionFrm(this);
        _stackWdt->addWidget(_pSessionFrm);
    }

    _stackWdt->setMinimumWidth(260);
    _stackWdt->setCurrentIndex(0);
}

/**
  * @函数名
  * @功能描述
  * @参数
  * @author cc
  * @date 2018.9.20
  */
void NavigationMainPanel::initMessage() {
    if (!_messageListener) {
        _messageListener = new NavigationMsgListener(this);
    }
}

/**
  * @功能描述
  * @参数
  * @author cc
  * @date 2018.9.18
  */
void NavigationMainPanel::connects() {
    qRegisterMetaType<QTalk::Entity::UID>("QTalk::Entity::UID");

    qRegisterMetaType<QTalk::StGroupInfo>("QTalk::StGroupInfo");
    qRegisterMetaType<std::map<std::string, std::string>>("std::map<std::string,std::string>");
    qRegisterMetaType<ReceiveSession>("ReceiveSession");
    qRegisterMetaType<StSessionInfo>("StSessionInfo");

    connect(_pSessionFrm, SIGNAL(sgSessionInfo(StSessionInfo)),
            this, SIGNAL(sgSessionInfo(StSessionInfo)));
    connect(this, &NavigationMainPanel::sgReceiveSession, _pSessionFrm, &SessionFrm::onReceiveSession);
    connect(this, SIGNAL(sgUpdateOnline()), _pSessionFrm, SLOT(onUpdateOnline()));
    connect(this, SIGNAL(sgUpdateOnlineUsers(std::map<std::string, std::string>)),
            _pSessionFrm, SLOT(onUpdateOnlineUsers(std::map<std::string, std::string>)));
    connect(this, SIGNAL(sgDownLoadHeadPhotosFinish()), _pSessionFrm, SLOT(onDownLoadHeadPhotosFinish()));
    connect(this, SIGNAL(sgDownLoadGroupHeadPhotosFinish()), _pSessionFrm, SLOT(onDownLoadGroupHeadPhotosFinish()));
    connect(this, &NavigationMainPanel::setDisconnectWgtVisible, _pTcpDisconnect, &TcpDisconnect::setVisible);
    connect(_pSessionFrm, &SessionFrm::showUserCard, this, &NavigationMainPanel::onSendShowCardSigal);
    connect(this, &NavigationMainPanel::updateGroupInfoSignal, _pSessionFrm, &SessionFrm::onUpdateGroupInfo);
    connect(this, &NavigationMainPanel::updateReadedCountSignal, _pSessionFrm, &SessionFrm::onUpdateReadedCount);
    connect(this, &NavigationMainPanel::recvRevokeMessageSignal, _pSessionFrm, &SessionFrm::recvRevikeMessage);
    connect(this, &NavigationMainPanel::loadSession, _pSessionFrm, &SessionFrm::onloadSessionData);
    qRegisterMetaType<QTalk::Entity::UID>("QTalk::Entity::UID");
    connect(_pSessionFrm, &SessionFrm::removeSession, this, &NavigationMainPanel::removeSession);
    connect(_pSessionFrm, &SessionFrm::removeSession, this, &NavigationMainPanel::removeSessionAction);
    connect(this, &NavigationMainPanel::destoryGroupSignal, _pSessionFrm, &SessionFrm::onDestroyGroup);
    connect(this, &NavigationMainPanel::sgShortCutSwitchSession, _pSessionFrm, &SessionFrm::onShortCutSwitchSession);
    connect(this, &NavigationMainPanel::sgChangeUserHead, _pSessionFrm, &SessionFrm::onUserHeadChange);
    connect(this, &NavigationMainPanel::sgUserConfigChanged, _pSessionFrm, &SessionFrm::onUserConfigChanged);
    //
    connect(this, &NavigationMainPanel::sgShowDraft, _pSessionFrm, &SessionFrm::onShowDraft);

    connect(this, &NavigationMainPanel::sgGotMState, _pSessionFrm, &SessionFrm::onGotMState);
}

///**
//  * @函数名
//  * @功能描述
//  * @参数
//  * @author   cc
//  * @date     2018/10/26
//  */
//void NavigationMianPanel::connToServerTimerSlot(bool sts) {
//    Q_ASSERT(PLAT.isMainThread());
//    if (nullptr != _pConnToServerTimer) {
//        sts ? _pConnToServerTimer->start() : _pConnToServerTimer->stop();
//    }
//}

/**
  * @函数名
  * @功能描述
  * @参数
  * @author cc
  * @date 2018.9.29
  */
void NavigationMainPanel::onDownLoadHeadPhotosFinish() {
    emit sgDownLoadHeadPhotosFinish();
}

/**
  * @函数名
  * @功能描述
  * @参数
  * @author cc
  * @date 2018.9.30
  */
void NavigationMainPanel::onDownLoadGroupHeadPhotosFinish() {
    emit sgDownLoadGroupHeadPhotosFinish();
}

/**
  * @函数名
  * @功能描述
  * @参数
  * @author cc
  * @date 2018.10.12
  */
void NavigationMainPanel::onUpdateOnline() {
    emit sgUpdateOnline();
}

/**
  * @函数名
  * @功能描述
  * @参数
  * @author cc
  * @date 2018.10.15
  */
void NavigationMainPanel::onUpdateOnlineUsers(const std::map<std::string, std::string> &userstatus) {
    emit sgUpdateOnlineUsers(userstatus);
}

/**
  * @函数名   onTcpDisconnect
  * @功能描述 断线处理
  * @参数
  * @author   cc
  * @date     2018/10/23
  */
void NavigationMainPanel::onTcpDisconnect() {
    _conneted = false;
    emit setDisconnectWgtVisible(true);
//    emit connToServerTimerSignal(true);
    // 立即重连一次
//    retryToConnect();
}

/**
  * @函数名   retryToConnect
  * @功能描述 触发重连
  * @参数
  * @author   cc
  * @date     2018/10/24
  */
//void NavigationMianPanel::retryToConnect() {
//
//    static qint64 call_time = 0;
//    qint64 now = QDateTime::currentDateTime().toMSecsSinceEpoch();
//    if(now - call_time < 5000)
//        return;
//    call_time = now;
//
//    if (PLAT.isMainThread() &&
//        nullptr != _pConnToServerTimer &&
//        _pConnToServerTimer->isActive()) {
//
//        info_log("retryToConnect ...");
//        _pTcpDisconnect->setText("正在重连");
//        _pConnToServerTimer->stop();
//    }
//
//    // check server host
//    const std::string host = NavigationManager::instance().getXmppHost();
//    const int port = NavigationManager::instance().getProbufPort();
//    if (port == 0 || host.empty()) {
//        warn_log("nav info error (port == 0 || domain.empty())");
////        emit connToServerTimerSignal(true);
//        return;
//    }
//    //
//    if (nullptr != _messageManager) {
//        QtConcurrent::run([this, host, port](){
//            // try connect to server
//            std::unique_ptr<QTcpSocket> tcpSocket(new QTcpSocket);
//            tcpSocket->connectToHost(host.data(), port);
//            if(!tcpSocket->waitForConnected(5000))
//            {
//                info_log("connect refuse by socket");
//                tcpSocket->abort();
////                emit connToServerTimerSignal(true);
//                if(_pTcpDisconnect)
//                    emit _pTcpDisconnect->sgSetText(tr("当前网络不可用"));
//                return;
//            }
//            tcpSocket->close();
//            // 重连
//            NavigationMsgManager::retryConnecToServer();
//        });
//    }
//}

/**
  * @函数名
  * @功能描述
  * @参数
  * @author cc
  * @date 2018.11.08
  */
void NavigationMainPanel::onNewSession(const StSessionInfo &into) {
    if (_pSessionFrm) {
        if (!PLAT.isMainThread()) {
            throw std::runtime_error("not main thread");
        }
        _pSessionFrm->onNewSession(into);
    }
}

///**
//  * @函数名   onRetryConnected
//  * @功能描述
//  * @参数
//  * @author   cc
//  * @date     2018/10/24
//  */
//void NavigationMianPanel::onRetryConnected() {
//    if (nullptr != _pTcpDisconnect) {
//        _pTcpDisconnect->onRetryConnected();
//        emit connToServerTimerSignal(true);
//    }
//}

/**
  * @函数名   onLoginSuccess
  * @功能描述 
  * @参数
  * @author   cc
  * @date     2018/10/24
  */
void NavigationMainPanel::onLoginSuccess() {
    {
        QMutexLocker locker(&_pSessionFrm->_mutex);
//        _pSessionFrm->pSessions = DB_PLAT.QueryImSessionInfos();
        _conneted = true;
//        emit loadSession();
    }
    emit setDisconnectWgtVisible(false);
    QtConcurrent::run([](){
        //refresh data
        NavigationMsgManager::getSessionData();
    });

//    emit connToServerTimerSignal(false);
}

void NavigationMainPanel::onSendShowCardSigal(const QString &userId) {
    emit showUserCardSignal(userId);
}

//
void NavigationMainPanel::onUpdateGroupInfo(std::shared_ptr<QTalk::StGroupInfo> info) {
    emit updateGroupInfoSignal(*info);
}

void NavigationMainPanel::updateReadCount(const QTalk::Entity::UID& uid, const int &count) {
    emit updateReadedCountSignal(uid, count);
}

void NavigationMainPanel::recvRevokeMessage(const QTalk::Entity::UID& uid, const std::string &from) {
    emit recvRevokeMessageSignal(uid, QString::fromStdString(from));
}

void NavigationMainPanel::onUpdateUserConfig(const std::vector<QTalk::Entity::ImConfig> &arConfigs) {

    if (nullptr == _pSessionFrm) {
        return;
    }
    //
    std::map<std::string, std::string> tmpStick;
    std::map<std::string, std::string> tmpNotice;
    std::vector<std::string> arSatr;
    std::vector<std::string> arBlackList;
    auto it = arConfigs.begin();
    for (; it != arConfigs.end(); it++) {
        //debug_log("onUpdateUserConfig {0} {1} {2}", it->ConfigKey, it->ConfigSubKey, it->ConfigValue);
        if (it->ConfigKey == "kStickJidDic") {
            QString xmppId = QString::fromStdString(it->ConfigSubKey);
            tmpStick[xmppId.toStdString()] = it->ConfigValue;
        } else if (it->ConfigKey == "kNoticeStickJidDic") {
            tmpNotice[it->ConfigSubKey] = it->ConfigValue;
        } else if (it->ConfigKey == "kStarContact") {
            arSatr.push_back(it->ConfigSubKey);
        } else if (it->ConfigKey == "kBlackList") {
            arBlackList.push_back(it->ConfigSubKey);
        }
    }
    //
    {
        QMutexLocker locker(&_pSessionFrm->_mutex);
        _pSessionFrm->_mapStick = tmpStick;
        _pSessionFrm->_mapNotice = tmpNotice;
        _pSessionFrm->_arSatr = arSatr;
        _pSessionFrm->_arBlackList = arBlackList;
        //
        _pSessionFrm->pSessions = DB_PLAT.reloadSession();
    }
    emit loadSession();
}

void NavigationMainPanel::onUpdateUserConfig(const std::map<std::string, std::string> &deleteData,
                                             const std::vector<QTalk::Entity::ImConfig>& arImConfig)
{
    QMutexLocker locker(&_pSessionFrm->_mutex);
    for(const auto& it : deleteData)
    {
        QTalk::Entity::UID uid(it.first);
        //
        if(it.second == "kStickJidDic")
        {
            _pSessionFrm->_mapStick.erase(it.first);
        }
        else if(it.second == "kNoticeStickJidDic")
        {
            _pSessionFrm->_mapNotice.erase(it.first);
        }
        else if(it.second == "kStarContact")
        {
            auto itFind = std::find(_pSessionFrm->_arSatr.begin(), _pSessionFrm->_arSatr.end(), it.first);
            if(itFind != _pSessionFrm->_arSatr.end())
                _pSessionFrm->_arSatr.erase(itFind);
        }
        else if(it.second == "kBlackList")
        {
            auto itFind = std::find(_pSessionFrm->_arBlackList.begin(), _pSessionFrm->_arBlackList.end(), it.first);
            if(itFind != _pSessionFrm->_arBlackList.end())
                _pSessionFrm->_arBlackList.erase(itFind);
        }
        else {}
        //
        emit sgUserConfigChanged(uid);
    }
    //
    for(const auto& conf : arImConfig)
    {
        QTalk::Entity::UID uid(conf.ConfigSubKey);

        if(conf.ConfigKey == "kStickJidDic")
            _pSessionFrm->_mapStick[conf.ConfigSubKey] = conf.ConfigValue;
        else if(conf.ConfigKey == "kNoticeStickJidDic")
            _pSessionFrm->_mapNotice[conf.ConfigSubKey] = conf.ConfigValue;
        else if(conf.ConfigKey == "kStarContact")
            _pSessionFrm->_arSatr.push_back(conf.ConfigSubKey);
        else if(conf.ConfigKey == "kBlackList")
            _pSessionFrm->_arBlackList.push_back(conf.ConfigSubKey);
        else {}
        //
        emit sgUserConfigChanged(uid);
    }
}

void NavigationMainPanel::onDestroyGroup(const std::string &groupId) {
    emit destoryGroupSignal(QString::fromStdString(groupId));
}

/**
 *
 */
void NavigationMainPanel::jumpToNewMessage() {
    if (!PLAT.isMainThread()) {
//        throw std::runtime_error("not main thread");
        return;
    }
    if (_pSessionFrm)
        _pSessionFrm->jumpToNewMessage();
}

/**
 *
 */
void NavigationMainPanel::onShortCutSwitchSession(int key) {
    emit sgShortCutSwitchSession(key);
}

void NavigationMainPanel::removeSessionAction(const QTalk::Entity::UID& uid) {
    std::string peerIdName = uid.usrId();
    NavigationMsgManager::removeSession(peerIdName);
}

/**
 *
 * @param ret
 * @param localHead
 */
void NavigationMainPanel::onChangeHeadRet(bool ret, const std::string& xmppId, const std::string &localHead)
{
    if(ret)
    {
        emit sgChangeUserHead(QString::fromStdString(xmppId), QString::fromStdString(localHead));
    }
}

void NavigationMainPanel::updateTatalReadCount()
{
    emit updateTotalUnreadCount(_pSessionFrm->getAllCount());
}

void NavigationMainPanel::onAppDeactivated() {
    if (_pSessionFrm)
        _pSessionFrm->onAppDeactivated();
}

/**
 *
 */
void NavigationMainPanel::onAppActive() {
    if (_pSessionFrm)
        _pSessionFrm->onAppActive();
}

void NavigationMainPanel::onGotMState(const QTalk::Entity::UID &uid, const QString &messageId, const long long &time) {
    emit sgGotMState(uid, messageId, time);
}

bool NavigationMainPanel::eventFilter(QObject *o, QEvent *e) {
    return QObject::eventFilter(o, e);
}
