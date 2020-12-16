 package com.nexstreaming.nexwebview.localProtocol;

 import android.net.LocalSocket;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;

/**
 * Created by jeongwook.yoon on 2016-10-05.
 */
public class nexProtocol {
    public static final int PACKET_SIGNATURE = 0x4e657850; //'NexP'
    public static final int PACKET_HEADER_SIZE = 16; //sifna, cmd , type, size
    public static final int PACKET_MAX_CHUNK_SIZE = 4096;

    public static class Header{
        public boolean isOk;
        public boolean isRequest;
        public int command;
        public int randsum;
        public int errorCode;
        public int messageSize;
    };

    public static class HDRnData extends Header{
        HDRnData(Header hdr){
            isOk = hdr.isOk;
            isRequest = hdr.isRequest;
            command = hdr.command;
            randsum = hdr.randsum;
            errorCode = hdr.errorCode;
            messageSize = hdr.messageSize;
        }
        public byte[] data;
    }

    public static byte[] makeSenderHeader(int command , int messageSize){
        final ByteBuffer handshake = ByteBuffer.allocate(PACKET_HEADER_SIZE);
        handshake.putInt(PACKET_SIGNATURE);
        handshake.putInt(command);
        handshake.putInt(0);
        handshake.putInt(messageSize);
        return handshake.array();
    }

    public static byte[] makeRequestHeader(short command, int SessionID  ,int messageSize){
        final ByteBuffer handshake = ByteBuffer.allocate(PACKET_HEADER_SIZE);
        handshake.putInt(PACKET_SIGNATURE);
        handshake.putChar('Q');
        handshake.putShort(command);
        handshake.putInt(SessionID);
        handshake.putInt(messageSize);
        return handshake.array();
    }

    public static byte[] makeResponseHeader(short command, int errorCode  ,int messageSize){
        final ByteBuffer handshake = ByteBuffer.allocate(PACKET_HEADER_SIZE);
        handshake.putInt(PACKET_SIGNATURE);
        handshake.putChar('S');
        handshake.putShort(command);
        handshake.putInt(errorCode);
        handshake.putInt(messageSize);
        return handshake.array();
    }

    public static void receive(LocalSocket socket, byte[] data, int len) throws IOException {
        final InputStream is = socket.getInputStream();
        int received = 0;
        while (received != len) {
            final int chunk = is.read(data, received, len - received);
            if (chunk < 0) {
                throw new IOException(
                        "I/O failure while receiving SDK controller data from socket.");
            }
            received += chunk;
        }
    }
    
    public static Header checkHeader(ByteBuffer buff){
        Header hdr = new Header();

        final int signature = buff.getInt();
        if(  signature != PACKET_SIGNATURE){
            return hdr;
        }
        hdr.isOk = true;
        hdr.command = buff.getInt();
        hdr.randsum = buff.getInt();
        hdr.messageSize = buff.getInt();
        return hdr;
    }

    public static Header checkHeader(ByteBuffer buff, boolean req ){
        Header hdr = new Header();

        final int signature = buff.getInt();
        if(  signature != PACKET_SIGNATURE){
            hdr.errorCode = nexProtoErrorCode.InvalidHDR.getValue();
            return hdr;
        }

        char c = buff.getChar();

        if(  c == 'Q' ){
            hdr.isRequest = true;
            if( !req ){
                hdr.errorCode = nexProtoErrorCode.InvalidRQ.getValue();
                return hdr;
            }
        }else if( c == 'S' ){
            hdr.isRequest = false;
            if( req ){
                hdr.errorCode = nexProtoErrorCode.InvalidRS.getValue();
                return hdr;
            }
        }

        hdr.command = buff.getShort();
        if( req ) {
            hdr.randsum = buff.getInt();
        }else{
            hdr.errorCode = buff.getInt();
        }
        hdr.isOk = true;
        hdr.messageSize = buff.getInt();
        return hdr;
    }
}
