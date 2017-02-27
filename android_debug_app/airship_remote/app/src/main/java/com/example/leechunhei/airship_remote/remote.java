package com.example.leechunhei.airship_remote;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;

import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.os.AsyncTask;

import java.io.IOException;
import java.util.UUID;

import static com.example.leechunhei.airship_remote.MainActivity.EXTRA_ADDRESS;

import org.jinkia.jk_rockerview.RockerView;

import static org.jinkia.jk_rockerview.RockerView.DirectionMode.DIRECTION_2_VERTICAL;
import static org.jinkia.jk_rockerview.RockerView.DirectionMode.DIRECTION_8;



public class remote extends AppCompatActivity {

    Button disconnect;
    RockerView joystick;
    RockerView joystickVertical;

    private TextView motor1Speed;
    private TextView motor2Speed;
    private TextView motor3Speed;

    int dir;
    int verticalDir;
    int speedLevel;
    int verticalSpeedLevel;
    double degree;
    double percentage;

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
        setContentView(R.layout.activity_remote);

        //call the widgtes
        disconnect=(Button) findViewById(R.id.disconnect);

        new ConnectBT().execute(); //Call the class to connect

        //commands to be sent to bluetooth

        joystick = (RockerView) findViewById(R.id.joystick);
        joystickVertical=(RockerView) findViewById(R.id.joystickvertical);

        motor1Speed = (TextView) findViewById(R.id.motor1);
        motor2Speed = (TextView) findViewById(R.id.direction);
        motor3Speed = (TextView) findViewById(R.id.motor3);

        joystick.setOnShakeListener(DIRECTION_8, new RockerView.OnShakeListener() {
            @Override
            public void onStart() {

            }

            @Override
            public void direction(RockerView.Direction direction) {
                if (direction == RockerView.Direction.DIRECTION_CENTER){
                    dir=0;//中心
                    motorControl(1,0);
                    motorControl(2,0);
                    motorControl(3,0);
                }else if (direction == RockerView.Direction.DIRECTION_DOWN){
                    dir=7;//下
                    motorControl(1,0-speedLevel);
                    motorControl(2,0-speedLevel);
                }else if (direction == RockerView.Direction.DIRECTION_LEFT){
                    dir=5;//左
                    motorControl(1,speedLevel);
                    motorControl(2,0-speedLevel);
                }else if (direction == RockerView.Direction.DIRECTION_UP){
                    dir=3;//上
                    motorControl(1,speedLevel);
                    motorControl(2,speedLevel);
                }else if (direction == RockerView.Direction.DIRECTION_RIGHT){
                    dir=1;//右
                    motorControl(1,0-speedLevel);
                    motorControl(2,speedLevel);
                }else if (direction == RockerView.Direction.DIRECTION_DOWN_LEFT){
                    dir=6;//左下
                    percentage=(degree-112.5)/45;
                    motorControl(1,(int)(2*speedLevel*percentage-speedLevel));
                    motorControl(2,speedLevel);
                }else if (direction == RockerView.Direction.DIRECTION_DOWN_RIGHT){
                    dir=8;//右下
                    percentage=(degree-22.5)/45;
                    motorControl(1,0-speedLevel);
                    motorControl(2,(int)(speedLevel-2*speedLevel*percentage));
                }else if (direction == RockerView.Direction.DIRECTION_UP_LEFT){
                    dir=4;//左上
                    percentage=(degree-202.5)/45;
                    motorControl(1,speedLevel);
                    motorControl(2,(int)(2*speedLevel*percentage-speedLevel));
                }else if (direction == RockerView.Direction.DIRECTION_UP_RIGHT){
                    dir=2;//右上
                    percentage=(degree-292.5)/45;
                    motorControl(1,(int)(speedLevel-2*speedLevel*percentage));
                    motorControl(2,speedLevel);
                }
            }

            @Override
            public void onFinish() {

            }
        });
        joystick.setOnAngleChangeListener(new RockerView.OnAngleChangeListener() {
            @Override
            public void onStart() {

            }

            @Override
            public void angle(double angle) {
                degree=angle;
                switch (dir){
                    case 2:
                        percentage=(degree-292.5)/45;
                        motorControl(1,(int)(speedLevel-2*speedLevel*percentage));
                        motorControl(2,speedLevel);
                        break;
                    case 4:
                        percentage=(degree-202.5)/45;
                        motorControl(1,speedLevel);
                        motorControl(2,(int)(2*speedLevel*percentage-speedLevel));
                        break;
                    case 6:
                        percentage=(degree-112.5)/45;
                        motorControl(1,(int)(2*speedLevel*percentage-speedLevel));
                        motorControl(2,speedLevel);
                        break;
                    case 8:
                        percentage=(degree-22.5)/45;
                        motorControl(1,0-speedLevel);
                        motorControl(2,(int)(speedLevel-2*speedLevel*percentage));
                        break;
                }
            }

            @Override
            public void onFinish() {

            }
        });

        joystick.setOnDistanceLevelListener(new RockerView.OnDistanceLevelListener() {
            @Override
            public void onDistanceLevel(int level) {
                speedLevel=225/9*level;
                switch (dir){
                    case 0:
                        motorControl(1,0);
                        motorControl(2,0);
                        break;
                    case 1:
                        motorControl(1,0-speedLevel);
                        motorControl(2,speedLevel);
                        break;
                    case 2:
                        motorControl(1,(int)(speedLevel-2*speedLevel*percentage));
                        motorControl(2,speedLevel);
                        break;
                    case 3:
                        motorControl(1,speedLevel);
                        motorControl(2,speedLevel);
                        break;
                    case 4:
                        motorControl(1,speedLevel);
                        motorControl(2,(int)(2*speedLevel*percentage-speedLevel));
                        break;
                    case 5:
                        motorControl(1,speedLevel);
                        motorControl(2,0-speedLevel);
                        break;
                    case 6:
                        motorControl(1,(int)(2*speedLevel*percentage-speedLevel));
                        motorControl(2,speedLevel);
                        break;
                    case 7:
                        motorControl(1,0-speedLevel);
                        motorControl(2,0-speedLevel);
                        break;
                    case 8:
                        motorControl(1,0-speedLevel);
                        motorControl(2,(int)(speedLevel-2*speedLevel*percentage));
                        break;
                }
                //mTvLevel.setText(speedLevel);
            }
        });

        joystickVertical.setOnShakeListener(DIRECTION_2_VERTICAL, new RockerView.OnShakeListener() {
            @Override
            public void onStart() {

            }

            @Override
            public void direction(RockerView.Direction direction) {
                if (direction == RockerView.Direction.DIRECTION_CENTER){
                    verticalDir=0;//中心
                    motorControl(3,0);
                }else if (direction == RockerView.Direction.DIRECTION_DOWN){
                    verticalDir=2;//下
                    motorControl(3,0-verticalSpeedLevel);
                }else if (direction == RockerView.Direction.DIRECTION_UP){
                    verticalDir=1;//上
                    motorControl(3,verticalSpeedLevel);
                }
            }

            @Override
            public void onFinish() {

            }
        });

        joystickVertical.setOnDistanceLevelListener(new RockerView.OnDistanceLevelListener() {
            @Override
            public void onDistanceLevel(int level) {
                verticalSpeedLevel=225/9*level;
                switch (verticalDir){
                    case 0:
                        motorControl(3,0);
                        break;
                    case 1:
                        motorControl(3,verticalSpeedLevel);
                        break;
                    case 2:
                        motorControl(3,0-verticalSpeedLevel);
                        break;
                }
                //mTvLevel.setText(speedLevel);
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
                motor1Speed.setText(String.valueOf(speed));
                break;
            case 2:
                motor2Speed.setText(String.valueOf(speed));
                break;
            case 3:
                motor3Speed.setText(String.valueOf(speed));
                break;
        }
        if (btSocket!=null)
        {
            String buff="M"+Character.toString((char)motorNumber);
            if(speed>0){
                buff += "C";
            }else{
                buff += "A";
                speed=-1*speed;
            }
            buff += Character.toString((char)speed);
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
            progress = ProgressDialog.show(remote.this, "Connecting...", "Please wait!!!");  //show a progress dialog
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
