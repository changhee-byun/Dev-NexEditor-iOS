package com.nexstreaming.app.common.localprotocol;

import android.net.LocalServerSocket;
import android.net.LocalSocket;
import android.util.Log;

import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * Created by jeongwook.yoon on 2016-10-05.
 */
public class nexServer {
    private static final String TAG ="nexServer";
    private String mAddress;
    private LocalServerSocket mServerSocket;
    private onAcceptCallback mAccept;

    interface onAcceptCallback {
        void onProcess(LocalSocket sock, nexProtocol.Header hdr ) throws IOException;
    }

    public nexServer(String address, onAcceptCallback callback ){
        mAddress = address;
        mAccept = callback;
    }

    private void runIOLooper(){
        Log.d(TAG, "In nexServer I/O looper.");
        do {
            try {
                // Create non-blocking server socket that would listen for connections,
                // and bind it to the given port on the local host.
                mServerSocket = new LocalServerSocket(mAddress);
                LocalServerSocket socket = mServerSocket;
                while (socket != null) {
                    final LocalSocket sk = socket.accept();
                    if (mServerSocket != null) {
                            onAccept(sk);
                    } else {
                        break;
                    }
                    socket = mServerSocket;
                }
            } catch (IOException e) {
                Log.e(TAG,"Exception " + e + "nexServer I/O looper.");
            }
            Log.d(TAG, "Exiting nexServer I/O looper.");

            // If we're exiting the internal loop for reasons other than an explicit
            // disconnect request, we should reconnect again.
        } while (disconnect());
    }

    private void onAccept(LocalSocket sock) throws IOException {
        Log.d(TAG, "onAccept() start");
        final ByteBuffer recv = ByteBuffer.allocate(nexProtocol.PACKET_HEADER_SIZE);
        nexProtocol.receive(sock, recv.array(), recv.array().length);
        nexProtocol.Header hdr = nexProtocol.checkHeader(recv);
        if( !hdr.isOk ){
            Log.d(TAG, "invalid header! close");
            sock.close();
            return;
        }
        mAccept.onProcess(sock,hdr);
    }

    public void connect() {
        Log.d(TAG, "nexServer is connecting...");
        // Start connection listener.
        new Thread(new Runnable() {
            @Override
            public void run() {
                runIOLooper();
            }
        }, "nexServerIoLoop").start();
    }

    public boolean disconnect() {
        // This is the only place in this class where we will null the
        // socket object. Since this method can be called concurrently from
        // different threads, lets do this under the lock.
        LocalServerSocket socket;
        synchronized (this) {
            socket = mServerSocket;
            mServerSocket = null;
        }
        if (socket != null) {
            Log.d(TAG, "nexServer is stopping I/O looper...");
            // Stop accepting new connections.
            wakeIOLooper(socket);
            try {
                socket.close();
            } catch (Exception e) {
                Log.e(TAG,"Exception " + e + "disconnect.");
            }
        }
        return socket != null;
    }

    private void wakeIOLooper(LocalServerSocket socket) {
        // We wake the looper by connecting to the socket.
        LocalSocket waker = new LocalSocket();
        try {
            waker.connect(socket.getLocalSocketAddress());
        } catch (IOException e) {
            Log.e(TAG,"Exception " + e + " in nexServer while waking up the I/O looper.");
        }
    }


}
