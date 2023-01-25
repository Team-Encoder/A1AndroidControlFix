package com.android.SimpsonsControllerFix;

import android.util.Log;

import java.io.File;
import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;



public class SerialPort {
    private static final String TAG = "SerialPort";
    private FileDescriptor mFd;
    private FileInputStream mFileInputStream;
    private FileOutputStream mFileOutputStream;

    static {
        System.loadLibrary("serial_port");
    }

    public SerialPort(File device, int baudrate, int flags) throws SecurityException, IOException {
        FileDescriptor v0 = SerialPort.open(device.getAbsolutePath(), baudrate,flags);
        this.mFd = v0;
        if(v0 != null) {
            this.mFileInputStream = new FileInputStream(this.mFd);
            this.mFileOutputStream = new FileOutputStream(this.mFd);
            return;
        }

        Log.e("SerialPort", "native open returns null");
        throw new IOException();
    }

    public native void close();

    public InputStream getInputStream() {
        return this.mFileInputStream;
    }

    public OutputStream getOutputStream() {
        return this.mFileOutputStream;
    }

    private static native FileDescriptor open(String path, int baudrate, int flags);


}
