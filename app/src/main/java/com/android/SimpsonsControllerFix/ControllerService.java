package com.android.SimpsonsControllerFix;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.os.Binder;
import android.os.Build;
import android.os.IBinder;
import android.os.RemoteCallbackList;
import android.util.Log;

import androidx.annotation.Nullable;
import androidx.annotation.RequiresApi;
import androidx.core.app.NotificationCompat;

import java.io.DataOutputStream;
import java.io.IOException;

public class ControllerService extends Service {

    final byte[] YM01;
    private final IBinder mBinder;
    private RemoteCallbackList mListenerList;

    public ControllerService()
    {
        this.mListenerList = new RemoteCallbackList();
        this.YM01 = new byte[]{89, 77, 0x30, 49};
        this.mBinder = new Binder();

    }

    @RequiresApi(api = Build.VERSION_CODES.O)
    @Override
    public void onCreate() {
        super.onCreate();
        //((NotificationManager)this.getSystemService("notification")).createNotificationChannel(new NotificationChannel("channel_uart", "前台服务", 4));
        //this.startForeground(1, new Notification.Builder(this, "channel_uart").build());
        //this.startForeground(1, new Notification());
        startMyOwnForeground();
    }


/*
((NotificationManager)this.getSystemService("notification")).createNotificationChannel(new NotificationChannel("channel_uart", "前台服务", 4));
        this.startForeground(1, new Notification.Builder(this, "channel_uart").build());
 */
    @RequiresApi(api = Build.VERSION_CODES.O)
    private void startMyOwnForeground()
    {
        String NOTIFICATION_CHANNEL_ID = "example.permanence";
        String channelName = "Background Service";

        NotificationChannel chan = new NotificationChannel(NOTIFICATION_CHANNEL_ID, channelName, NotificationManager.IMPORTANCE_NONE);
        chan.setLightColor(Color.BLUE);
        chan.setLockscreenVisibility(Notification.VISIBILITY_PRIVATE);

        NotificationManager manager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
        assert manager != null;
        manager.createNotificationChannel(chan);

        NotificationCompat.Builder notificationBuilder = new NotificationCompat.Builder(this, NOTIFICATION_CHANNEL_ID);
        Notification notification = notificationBuilder.setOngoing(true)
                .setContentTitle("App is running in background")
                .setPriority(NotificationManager.IMPORTANCE_MIN)
                .setCategory(Notification.CATEGORY_SERVICE)
                .build();
        startForeground(2, notification);
    }

    class ControllerProcessThread extends Thread {
        private ControllerProcessThread(){}

        @Override
        public void run()
        {
            while(true)
            {
                try {

                    String libDir = getBaseContext().getApplicationInfo().nativeLibraryDir;
                    Log.i("Team-Encoder Fix"," libDir: " + libDir);

                    Process ControlProcess = Runtime.getRuntime().exec("sh");
                    DataOutputStream shellOutput = new DataOutputStream(ControlProcess.getOutputStream());
                    shellOutput.writeBytes(libDir+"/libcontrolfix.so > /sdcard/libcontrolfix_exec.log 2>&1\n");
                    shellOutput.flush();
                    shellOutput.writeBytes("sync\n");
                    shellOutput.flush();
                    ControlProcess.waitFor();

                } catch (IOException | InterruptedException e) {
                    e.printStackTrace();
                }
                try {
                    Thread.sleep(60000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }
    }


    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        super.onStartCommand(intent, flags, startId);

        Thread mThread = new ControllerProcessThread();
        mThread.start();

        return START_STICKY; // original return?
    }


    @Override
    public void onDestroy() {
        super.onDestroy();

        // old we don't want our service to restart anymore.
       // this.CloseUart();
       //stoptimertask();

       // Intent broadcastIntent = new Intent();
       // broadcastIntent.setAction("restartservice");
       // broadcastIntent.setClass(this, Restarter.class);
       // this.sendBroadcast(broadcastIntent);
    }

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }





}