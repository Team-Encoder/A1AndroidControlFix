package com.android.SimpsonsControllerFix;

import android.app.Activity;
import android.app.ActivityManager;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;

import java.io.File;


public class MainActivity extends Activity {
    public SerialPort serialPort;
    public SerialPort serialPort3;
    private byte[] keybuffer;
    Intent mServiceIntent;
    private ControllerService mControllerService;
    //private ReadThread mReadThread;




    class QueryKeyThread extends Thread {
        private final byte[] data;

        private QueryKeyThread(){
            this.data = new byte[] {-90,1,0};
        }

        QueryKeyThread(MainActivity x0){
            this.data = new byte[] {-90,1,0};
        }

        @Override
        public void run()
        {
           /* while(!MainActivity.this.bQuitQueryKey) {
                try{
                    MainActivity.this.mOutputStream.write(this.data);
                    Thread.sleep(10L);
                }
                catch(Exception e)
                {
                    e.printStackTrace();
                }
            }*/
        }
    }


    private void SendUart(byte[] data)
    {
        try{
            //this.mSolenoidOutStream.write(data);
        }
        catch(Exception e)
        {
            e.printStackTrace();
        }
    }

    public boolean CloseUart() {
        //this.bQuitQueryKey = true;
        try{
            Thread.sleep(10L);
        }catch(Exception e)
        {
            e.printStackTrace();
        }

        if(this.serialPort != null) {
            this.serialPort.close();
            this.serialPort = null;
        }
        return false;
    }



    private void Uart_Init()
    {
        try{
            SerialPort serial = new SerialPort(new File("/dev/ttyS1"), 115200, 0);
            this.serialPort = serial;
            //this.mInputStream = serial.getInputStream();
            //this.mOutputStream = serial.getOutputStream();
            //this.mReadThread = new ReadThread(this);
            //mReadThread.start();
        }
        catch(Exception e)
        {
            e.printStackTrace();
        }
    }

    private void Uart3_Init()
    {
        try {
            SerialPort srl = new SerialPort(new File("/dev/ttyS3"), 115200, 0);
            this.serialPort3 = srl;
            //this.mSolenoidInStream = srl.getInputStream();
            //this.mSolenoidOutStream = srl.getOutputStream();
        }
        catch(Exception e)
        {
            e.printStackTrace();
        }
    }

    private boolean isMyServiceRunning(Class<?> serviceClass) {

        ActivityManager manager = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
        for (ActivityManager.RunningServiceInfo service : manager.getRunningServices(Integer.MAX_VALUE)) {
            if (serviceClass.getName().equals(service.service.getClassName())) {
                Log.i ("Service status", "Running");
                return true;
            }
        }
        Log.i ("Service status", "Not running");
        return false;
    }


    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);

        if (! hasFocus) {
            Intent closeDialog = new Intent(Intent.ACTION_CLOSE_SYSTEM_DIALOGS);
            sendBroadcast(closeDialog);
        }
    }


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Log.i("Team-Encoder","If you look at this code, I feel bad for you it's junk.");
        Log.i("Team-Encoder","Simpsons Fix.... Danny Dawson was not here.");

        //this.Uart_Init();
        //this.Uart3_Init();

        // bind this to a key inside of service so that actions can be closed.
        Intent closeDialog = new Intent(Intent.ACTION_CLOSE_SYSTEM_DIALOGS);
        sendBroadcast(closeDialog);
    }


    @Override
    protected void onPause()
    {
        //CloseUart();
        try {
            Thread.sleep(100L);
        }
        catch(Exception exception)
        {
            exception.printStackTrace();
        }

        //SerialPort srl = this.serialPort3;
        //if(srl != null)
        //{
         //   serialPort3.close();
          //  this.serialPort3 = null;
       // }

        Log.d("APP","onPause");
        System.exit(1);
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        //this.CloseUart();
       // SerialPort srl = this.serialPort3;
        //if(srl != null) {
        //    serialPort3.close();
        //    this.serialPort3 = null;
        //}
        super.onDestroy();
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        return true;
    }

}
