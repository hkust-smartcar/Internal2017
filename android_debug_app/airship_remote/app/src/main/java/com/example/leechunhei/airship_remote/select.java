package com.example.leechunhei.airship_remote;

import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import java.io.IOException;
import java.util.UUID;

import static com.example.leechunhei.airship_remote.MainActivity.EXTRA_ADDRESS;


public class select extends AppCompatActivity {

    Button remote,smartCarRemote;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        Intent newint = getIntent();
        setContentView(R.layout.activity_select);
        remote=(Button)findViewById(R.id.remote);
        smartCarRemote=(Button)findViewById(R.id.smartCarRemote);

        remote.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent i = new Intent(select.this, MainActivity.class);
                startActivity(i);
            }
        });

        smartCarRemote.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent i = new Intent(select.this, smartcarbluetoothpair.class);
                startActivity(i);
            }
        });
    }
}
