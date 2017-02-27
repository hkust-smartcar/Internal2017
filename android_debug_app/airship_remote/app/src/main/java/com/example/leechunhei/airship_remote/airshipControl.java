package com.example.leechunhei.airship_remote;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;

import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.view.View;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.os.AsyncTask;

import java.io.IOException;
import java.util.UUID;

import static com.example.leechunhei.airship_remote.MainActivity.EXTRA_ADDRESS;


public class airshipControl extends AppCompatActivity {

    Button motor1_clockwise, motor2_clockwise, motor3_clockwise,motor1_anticlockwise,motor2_anticlockwise,motor3_anticlockwise,disconnect,remote;
    SeekBar motor1,motor2,motor3;
    TextView motor1Speed, motor2Speed,motor3Speed;
    String address = null;
    private ProgressDialog progress;
    BluetoothAdapter myBluetooth = null;
    BluetoothSocket btSocket = null;
    private boolean isBtConnected = false;
    //SPP UUID. Look for it
    static final UUID myUUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        Intent newint = getIntent();
        address = newint.getStringExtra(EXTRA_ADDRESS); //receive the address of the bluetooth device

        //view of the ledControl
        setContentView(R.layout.activity_airship_control);

        //call the widgtes
        motor1_clockwise = (Button)findViewById(R.id.motor1_clockwise);
        motor2_clockwise = (Button)findViewById(R.id.motor2_clockwise);
        motor3_clockwise = (Button)findViewById(R.id.motor3_clockwise);
        motor1_anticlockwise=(Button) findViewById(R.id.motor1_anticlockwise);
        motor2_anticlockwise=(Button) findViewById(R.id.motor2_anticlockwise);
        motor3_anticlockwise=(Button) findViewById(R.id.motor3_anticlockwise);
        disconnect=(Button) findViewById(R.id.disconnect);
        remote=(Button) findViewById(R.id.remote);
        motor1 = (SeekBar) findViewById(R.id.motor1);
        motor2=(SeekBar) findViewById(R.id.direction);
        motor3=(SeekBar) findViewById(R.id.motor3);
        motor1Speed = (TextView)findViewById(R.id.motor1_speed);
        motor2Speed = (TextView)findViewById(R.id.motor2_speed);
        motor3Speed = (TextView)findViewById(R.id.motor3_speed);

        new ConnectBT().execute(); //Call the class to connect

        //commands to be sent to bluetooth
        motor1_clockwise.setOnClickListener(new View.OnClickListener()
        {
            @Override
            public void onClick(View v)
            {
                motorControl(1,450);
            }
        });

        motor2_clockwise.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v)
            {
                motorControl(2,450);
            }
        });

        motor3_clockwise.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v)
            {
                motorControl(3,450);
            }
        });

        motor1_anticlockwise.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v)
            {
                motorControl(1,0);
            }
        });

        motor2_anticlockwise.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v)
            {
                motorControl(2,0);
            }
        });

        motor3_anticlockwise.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v)
            {
                motorControl(3,0);
            }
        });

        disconnect.setOnClickListener(new View.OnClickListener()
        {
            @Override
            public void onClick(View v)
            {
                Disconnect(); //close connection
            }
        });

        remote.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent i = new Intent(airshipControl.this, remote.class);
                startActivity(i);
            }
        });

        motor1.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser==true)
                {
                    motorControl(1,progress);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        motor2.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser==true)
                {
                    motorControl(2,progress);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        motor3.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser==true)
                {
                    motorControl(3,progress);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });
    }

    private void Disconnect()
    {
        if (btSocket!=null) //If the btSocket is busy
        {
            try
            {
                btSocket.close(); //close connection
            }
            catch (IOException e)
            { msg("Error");}
        }
        finish(); //return to the first layout

    }

    private void motorControl(int motorNumber,int speed)
    {
        switch (motorNumber){
            case 1:
                motor1.setProgress(speed);
                break;
            case 2:
                motor2.setProgress(speed);
                break;
            case 3:
                motor3.setProgress(speed);
                break;
        }
        if (btSocket!=null)
        {
            String buff="M"+Character.toString((char)motorNumber);
            speed-=225;
            if(speed>0){
                buff += "C";
            }else{
                buff += "A";
                speed=-1*speed;
            }
            buff += Character.toString((char)speed);
            switch (motorNumber){
                case 1:
                    motor1Speed.setText(String.valueOf(speed));
                    break;
                case 2:
                    motor2Speed.setText(String.valueOf(speed));
                    break;
                case 3:
                    motor3Speed.setText(String.valueOf(speed));
            }
            try
            {
                btSocket.getOutputStream().write(buff.getBytes());
            }
            catch (IOException e)
            {
                msg("Error");
            }
        }
    }

    // fast way to call Toast
    private void msg(String s)
    {
        Toast.makeText(getApplicationContext(),s,Toast.LENGTH_LONG).show();
    }

    private class ConnectBT extends AsyncTask<Void, Void, Void>  // UI thread
    {
        private boolean ConnectSuccess = true; //if it's here, it's almost connected

        @Override
        protected void onPreExecute()
        {
            progress = ProgressDialog.show(airshipControl.this, "Connecting...", "Please wait!!!");  //show a progress dialog
        }

        @Override
        protected Void doInBackground(Void... devices) //while the progress dialog is shown, the connection is done in background
        {
            try
            {
                if (btSocket == null || !isBtConnected)
                {
                    myBluetooth = BluetoothAdapter.getDefaultAdapter();//get the mobile bluetooth device
                    BluetoothDevice dispositivo = myBluetooth.getRemoteDevice(address);//connects to the device's address and checks if it's available
                    btSocket = dispositivo.createInsecureRfcommSocketToServiceRecord(myUUID);//create a RFCOMM (SPP) connection
                    BluetoothAdapter.getDefaultAdapter().cancelDiscovery();
                    btSocket.connect();//start connection
                }
            }
            catch (IOException e)
            {
                ConnectSuccess = false;//if the try failed, you can check the exception here
            }
            return null;
        }
        @Override
        protected void onPostExecute(Void result) //after the doInBackground, it checks if everything went fine
        {
            super.onPostExecute(result);

            if (!ConnectSuccess)
            {
                msg("Connection Failed. Is it a SPP Bluetooth? Try again.");
                finish();
            }
            else
            {
                msg("Connected.");
                isBtConnected = true;
            }
            progress.dismiss();
        }
    }
}



/*
import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.os.AsyncTask;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.Toast;

import java.io.IOException;
import java.util.UUID;

public class airshipControl extends AppCompatActivity {

    private Button motor1_clockwise;
    private Button motor2_clockwise;
    private Button motor3_clockwise;
    private Button motor1_anticlockwise;
    private Button motor2_anticlockwise;
    private Button motor3_anticlockwise;
    private Button disconnect;

    private SeekBar motor1;
    private SeekBar motor2;
    private SeekBar motor3;

    String address = null;
    private ProgressDialog progress;
    BluetoothAdapter myBluetooth = null;
    BluetoothSocket btSocket = null;
    private boolean isBtConnected = false;
    static final UUID myUUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    private void msg(String s){
        Toast.makeText(getApplicationContext(),s,Toast.LENGTH_LONG).show();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_airship_control);

        motor1_clockwise=(Button) findViewById(R.id.motor1_clockwise);
        motor2_clockwise=(Button) findViewById(R.id.motor2_clockwise);
        motor3_clockwise=(Button) findViewById(R.id.motor3_clockwise);
        motor1_anticlockwise=(Button) findViewById(R.id.motor1_anticlockwise);
        motor2_anticlockwise=(Button) findViewById(R.id.motor2_anticlockwise);
        motor3_anticlockwise=(Button) findViewById(R.id.motor3_anticlockwise);
        disconnect=(Button) findViewById(R.id.disconnect);

        motor1=(SeekBar) findViewById(R.id.motor1);
        motor2=(SeekBar) findViewById(R.id.motor2);
        motor3=(SeekBar) findViewById(R.id.motor3);

        Intent newint = getIntent();
        address = newint.getStringExtra(MainActivity.EXTRA_ADDRESS);
        setContentView(R.layout.activity_airship_control);

        motor1_clockwise.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (btSocket != null) {
                    try{
                        btSocket.getOutputStream().write("M".getBytes());
                        btSocket.getOutputStream().write(1);
                        btSocket.getOutputStream().write("C".getBytes());
                        btSocket.getOutputStream().write(225);
                    }
                    catch (IOException e){
                        msg("Error");
                    }
                }
            }
        });

        motor2_clockwise.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (btSocket != null) {
                    try{
                        btSocket.getOutputStream().write("M".getBytes());
                        btSocket.getOutputStream().write(2);
                        btSocket.getOutputStream().write("C".getBytes());
                        btSocket.getOutputStream().write(225);
                    }
                    catch (IOException e){
                        msg("Error");
                    }
                }
            }
        });

        motor3_clockwise.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (btSocket != null) {
                    try{
                        btSocket.getOutputStream().write("M".getBytes());
                        btSocket.getOutputStream().write(3);
                        btSocket.getOutputStream().write("C".getBytes());
                        btSocket.getOutputStream().write(225);
                    }
                    catch (IOException e){
                        msg("Error");
                    }
                }
            }
        });

        motor1_anticlockwise.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (btSocket != null) {
                    try{
                        btSocket.getOutputStream().write("M".getBytes());
                        btSocket.getOutputStream().write(1);
                        btSocket.getOutputStream().write("A".getBytes());
                        btSocket.getOutputStream().write(225);
                    }
                    catch (IOException e){
                        msg("Error");
                    }
                }
            }
        });

        motor2_anticlockwise.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (btSocket != null) {
                    try{
                        btSocket.getOutputStream().write("M".getBytes());
                        btSocket.getOutputStream().write(2);
                        btSocket.getOutputStream().write("A".getBytes());
                        btSocket.getOutputStream().write(225);
                    }
                    catch (IOException e){
                        msg("Error");
                    }
                }
            }
        });

        motor3_anticlockwise.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (btSocket != null) {
                    try{
                        btSocket.getOutputStream().write("M".getBytes());
                        btSocket.getOutputStream().write(3);
                        btSocket.getOutputStream().write("A".getBytes());
                        btSocket.getOutputStream().write(225);
                    }
                    catch (IOException e){
                        msg("Error");
                    }
                }
            }
        });

        motor1.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser==true){
                    try
                    {
                        String buff="M"+Character.toString((char)1);
                        int speed=progress-225;
                        if(speed>0){
                            buff += "C";
                        }else{
                            buff += "A";
                            speed=-1*speed;
                        }
                        buff += Character.toString((char)speed);
                        btSocket.getOutputStream().write(buff.getBytes());
                    }
                    catch (IOException e){
                    }
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        motor2.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser==true){
                    try
                    {
                        String buff="M"+Character.toString((char)2);
                        int speed=progress-225;
                        if(speed>0){
                            buff += "C";
                        }else{
                            buff += "A";
                            speed=-1*speed;
                        }
                        buff += Character.toString((char)speed);
                        btSocket.getOutputStream().write(buff.getBytes());
                    }
                    catch (IOException e){
                    }
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        motor3.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser==true){
                    try
                    {
                        String buff="M"+Character.toString((char)3);
                        int speed=progress-225;
                        if(speed>0){
                            buff += "C";
                        }else{
                            buff += "A";
                            speed=-1*speed;
                        }
                        buff += Character.toString((char)speed);
                        btSocket.getOutputStream().write(buff.getBytes());
                    }
                    catch (IOException e){
                    }
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });


        disconnect.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (btSocket!=null){
                    try{
                        btSocket.close(); //close connection
                    }
                    catch (IOException e){msg("Error");}
                }
                finish();
            }
        });

    }

    private class ConnectBT extends AsyncTask<Void, Void, Void> {
        private boolean ConnectSuccess = true;
        @Override
        protected void onPreExecute(){
            progress = ProgressDialog.show(airshipControl.this, "Connecting...", "Please wait!!!");
        }
        @Override
        protected Void doInBackground(Void... devices){
            try{
                if (btSocket == null || !isBtConnected){
                    myBluetooth = BluetoothAdapter.getDefaultAdapter();
                    BluetoothDevice dispositivo = myBluetooth.getRemoteDevice(address);
                    btSocket = dispositivo.createInsecureRfcommSocketToServiceRecord(myUUID);
                    BluetoothAdapter.getDefaultAdapter().cancelDiscovery();
                    btSocket.connect();
                }
            }
            catch (IOException e){
                ConnectSuccess = false;
            }
            return null;
        }
        @Override
        protected void onPostExecute(Void result)
        {
            super.onPostExecute(result);
            if (!ConnectSuccess){
                msg("Connection Failed. Is it a SPP Bluetooth? Try again.");
                finish();
            }
            else{
                msg("Connected.");
                isBtConnected = true;
            }
            progress.dismiss();
        }
    }

}
*/
