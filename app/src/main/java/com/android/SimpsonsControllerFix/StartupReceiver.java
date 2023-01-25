package com.android.SimpsonsControllerFix;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.util.Log;

import androidx.annotation.RequiresApi;

public class StartupReceiver extends BroadcastReceiver {
    @RequiresApi(api = Build.VERSION_CODES.O)
    @Override
    public void onReceive(Context context, Intent i)
    {
        Log.i("Team-Encoder", "Broadcast android.intent.action.BOOT_COMPLETED received.");
        context.startForegroundService(new Intent(context, ControllerService.class));
    }
}
