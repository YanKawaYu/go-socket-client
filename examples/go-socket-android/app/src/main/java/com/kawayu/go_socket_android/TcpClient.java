package com.kawayu.go_socket_android;

import android.util.Log;

public class TcpClient {
    public static final int DIS_NONE = 0;
    public static final int DIS_RESET = 1;
    public static final int DIS_NET_CHANGE = 2;
    public static final int DIS_NET_LOST = 3;
    public static final int DIS_KICKOUT = 4;
    public static final int DIS_LOGOUT = 5;
    public static final int DIS_CONNECT_TIMEOUT = 6;
    public static final int DIS_SEND_REQ_TIMEOUT = 7;
    public static final int DIS_PING_TIMEOUT = 8;

    public static final int NET_TYPE_UNKNOWN = 0;
    public static final int NET_TYPE_WIFI = 1;
    public static final int NET_TYPE_MOBILE = 2;
    public static final int NET_TYPE_NO_NET = 3;

    public static final int CONN_IDLE = 0;
    public static final int CONN_CONNECTING = 1;
    public static final int CONN_CONNECTED = 2;
    public static final int CONN_DISCONNECTED = 3;
    public static final int CONN_FAILED = 4;
    public static final int CONN_KICKOUT = 5;

    public static final int ERR_CODE_NONE = 0;
    public static final int ERR_CODE_NOT_CONNECTED = 1;
    public static final int ERR_CODE_TIMEOUT = 2;
    public static final int ERR_CODE_LOSE_CONNECT = 3;

    public static final int SVR_DIS_CODE_NONE = 0;
    public static final int SVR_DIS_CODE_KICKOFF = 1;

    static {
        try {
            System.loadLibrary("c++_shared");
            System.loadLibrary("gotcp_shared");
        }catch (Throwable e) {
            Log.e("gotcp", "", e);
        }
    }

    public interface GetConnectInfoListener {
        String getConnectInfo();
    }

    public interface ConnStatusChangeListener {
        void onConnStatusChange(int connStatus);
    }

    public interface SendReqListener {
        void onSendReq(String type, String payload);
    }

    public interface SendRespListener {
        void onSendResp(int errCode, String response);
    }

    public interface ServerDisconnectListener {
        void onServerDisconnect(int svrDisCode);
    }

    public interface GetHttpDnsListener {
        String[] getHttpDns(String host);
    }

    public static native void initHostAndPort(String host, String backupIp, int port, boolean isTls, GetConnectInfoListener listener);

    public static native void makeSureConnected();

    public static native void disconnectAsync(int disCode, boolean shouldReconnect);

    public static native boolean send(String type, String payload, byte[] data, SendRespListener listener);

    public static native boolean sendNoReply(String type, String payload);

    public static native void setOnConnStatusChange(ConnStatusChangeListener listener);

    public static native void setOnSendReq(SendReqListener listener);

    public static native void setOnServerDisconnect(ServerDisconnectListener listener);

    public static native void setHttpDnsListener(GetHttpDnsListener listener);

    public static native void setNetType(int netType);

    public static native int getConnectStatus();
}
