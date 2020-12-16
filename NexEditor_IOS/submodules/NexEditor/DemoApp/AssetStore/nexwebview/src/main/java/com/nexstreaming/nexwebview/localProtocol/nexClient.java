package com.nexstreaming.nexwebview.localProtocol;


import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.util.Log;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.ClosedChannelException;

/**
 * Created by jeongwook.yoon on 2016-10-05.
 */
public class nexClient {
    private static final String TAG ="nexClient";
    private String mAddress;
    LocalSocket mSocket;

    public nexClient(String address){
        mAddress = address;
    }

    public boolean connect() {
        mSocket =  new LocalSocket();
        try {
            mSocket.connect(new LocalSocketAddress(mAddress));
        } catch (IOException e) {
            Log.e(TAG, "Exception " + e + " while connect Socket");
            return false;
        }
        return true;
    }

    public boolean disconnect(){
        LocalSocket socket;
        synchronized (this) {
            socket = mSocket;
            mSocket = null;
        }
        if (socket != null) {
            try {
                // Force all I/O to stop before closing the socket.
                socket.shutdownInput();
                socket.shutdownOutput();
                socket.close();
                Log.d(TAG, "Socket is closed");
                return true;
            } catch (IOException e) {
                Log.e(TAG,"Exception " + e + " while closing Socket");
            }
        }
        return false;
    }

    public boolean isConnected(){
        return mSocket != null;
    }

    public void sendMessage(int command , byte[] message) throws IOException{
        LocalSocket socket = mSocket;
        if (socket == null) {
            Log.w(TAG,"'send' request on closed Socket ");
            throw new ClosedChannelException();
        }

        socket.getOutputStream().write(nexProtocol.makeSenderHeader(command, message.length));
        socket.getOutputStream().write(message);
    }

    public byte[] receiveMessage(int command) throws IOException{

        LocalSocket socket = mSocket;
        if (socket == null) {
            Log.w(TAG,"'receive' request on closed Socket ");
            throw new ClosedChannelException();
        }
        final ByteBuffer recv = ByteBuffer.allocate(nexProtocol.PACKET_HEADER_SIZE);
        nexProtocol.receive(socket, recv.array(), recv.array().length);
        nexProtocol.Header hdr= nexProtocol.checkHeader(recv);
        if( !hdr.isOk ){
            Log.w(TAG,"'receive' Header is invalid. ");
            throw new ClosedChannelException();
        }

        if( command != hdr.command ){
            Log.w(TAG,"'receive' command is invalid. ");
            throw new ClosedChannelException();
        }

        byte[] message = new byte[hdr.messageSize];
        nexProtocol.receive(socket, message, message.length);
        return message;
    }
}
