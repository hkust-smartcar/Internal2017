package com.example.leechunhei.airship_remote;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.v4.app.FragmentActivity;
import android.support.v7.app.AppCompatActivity;
import android.text.Layout;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import org.jinkia.jk_rockerview.RockerView;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.UUID;

import static com.example.leechunhei.airship_remote.BluetoothChatService.STATE_CONNECTED;
import static com.example.leechunhei.airship_remote.smartcarbluetoothpair.EXTRA_ADDRESS;
import static org.jinkia.jk_rockerview.RockerView.DirectionMode.DIRECTION_8;

public class SmartCarRemote extends AppCompatActivity {

    Handler bluetoothIn;

    final int handlerState = 0;                        //used to identify handler message
    private BluetoothAdapter btAdapter = null;
    private BluetoothSocket btSocket = null;

    private ConnectedThread mConnectedThread;

    // SPP UUID service - this should work for most devices
    static final UUID BTMODULEUUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    // String for MAC address
    private static String address;

    Button disconnect;
    RockerView joystick;
    TextView direction;

    int camHeight=120;
    int camWidth=160;
    byte[] camArray= new byte[camHeight*camWidth/8];
    int[][] cam2DArray= new int[camHeight][camWidth];
    boolean isCamData=false;
    boolean CamDataEnd=true;
    int camPos=0;
    int dir=0;

    long prevTime;

    DrawView cameraDraw;
    LinearLayout cameraLayout;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_smart_car_remote);

        //Link the buttons and textViews to respective views



        bluetoothIn = new Handler() {
            public void handleMessage(Message msg) {
                if (msg.what == handlerState) {                                     //if message is what we want
                    byte[] bluetoothData = (byte[]) msg.obj;                               // msg.arg1 = bytes from connect thread
                    for(int i=0;i<msg.arg1;i++) {
                        if (bluetoothData[i] == 49 && CamDataEnd==true) {
//                            System.arraycopy(bluetoothData,i+1,camArray,camPos,msg.arg1-i);
//                            camPos+=msg.arg1;
                            isCamData = true;
                            CamDataEnd=false;
//                            if(camPos>camHeight*camWidth/8-1){
//                                isCamData=false;
//                                CamDataEnd=true;
//                                camPos=0;
//                                drawCameraImage();
//                            }
//                            break;
                        }else if(isCamData){
//                            System.arraycopy(bluetoothData,i,camArray,camPos,msg.arg1-i);
                            camArray[camPos]=bluetoothData[i];
                            camPos++;
//                            if(camPos>camHeight*camWidth/8-1){
//                                isCamData=false;
//                                CamDataEnd=true;
//                                camPos=0;
//                                drawCameraImage();
//                            }
//                            break;
                        }
                        if(camPos==camHeight*camWidth/8-1){
                            isCamData=false;
                            CamDataEnd=true;
                            camPos=0;
                            drawCameraImage();
                        }
                    }
                }
            }
        };

        btAdapter = BluetoothAdapter.getDefaultAdapter();       // get Bluetooth adapter

        //Get MAC address from DeviceListActivity via intent
        Intent intent = getIntent();

        //Get the MAC address from the DeviceListActivty via EXTRA
        address = intent.getStringExtra(EXTRA_ADDRESS);

        //create device and set the MAC address
        BluetoothDevice device = btAdapter.getRemoteDevice(address);

        try {
            btSocket=device.createInsecureRfcommSocketToServiceRecord(BTMODULEUUID);
            //btSocket = createBluetoothSocket(device);
        } catch (IOException e) {
            Toast.makeText(getBaseContext(), "Socket creation failed", Toast.LENGTH_LONG).show();
        }
        // Establish the Bluetooth socket connection.
        try
        {
            BluetoothAdapter.getDefaultAdapter().cancelDiscovery();
            btSocket.connect();
        } catch (IOException e) {
            try
            {
                btSocket.close();
            } catch (IOException e2)
            {
                //insert code to deal with this
            }
        }
        mConnectedThread = new ConnectedThread(btSocket);
        mConnectedThread.start();

        disconnect=(Button) findViewById(R.id.disconnect);
        joystick=(RockerView) findViewById(R.id.joystick);
        direction=(TextView) findViewById(R.id.direction);

        disconnect.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                onPause();
                finish();
            }
        });

        joystick.setOnShakeListener(DIRECTION_8, new RockerView.OnShakeListener() {
            @Override
            public void onStart() {
            }

            @Override
            public void direction(RockerView.Direction direction) {
                if (direction == RockerView.Direction.DIRECTION_CENTER) {
                    dir = 0;//中心
                } else if (direction == RockerView.Direction.DIRECTION_DOWN) {
                    dir = 7;//下
                } else if (direction == RockerView.Direction.DIRECTION_LEFT) {
                    dir = 5;//左
                } else if (direction == RockerView.Direction.DIRECTION_UP) {
                    dir = 3;//上
                } else if (direction == RockerView.Direction.DIRECTION_RIGHT) {
                    dir = 1;//右
                } else if (direction == RockerView.Direction.DIRECTION_DOWN_LEFT) {
                    dir = 6;//左下
                } else if (direction == RockerView.Direction.DIRECTION_DOWN_RIGHT) {
                    dir = 8;//右下
                } else if (direction == RockerView.Direction.DIRECTION_UP_LEFT) {
                    dir = 4;//左上
                } else if (direction == RockerView.Direction.DIRECTION_UP_RIGHT) {
                    dir = 2;//右上
                }
                control(dir);
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
//                degree = angle;
//                switch (dir) {
//                    case 2:
//                        anglePercentage=(degree-180)/180;
//                        break;
//                    case 3:
//                        anglePercentage=(degree-180)/180;
//                        break;
//                    case 4:
//                        anglePercentage=(degree-180)/180;
//                        break;
//                    case 6:
//                        anglePercentage=(degree)/180;
//                        break;
//                    case 7:
//                        anglePercentage=(degree)/180;
//                        break;
//                    case 8:
//                        anglePercentage=(degree)/180;
//                        break;
//                }
            }

            @Override
            public void onFinish() {

            }
        });

        joystick.setOnDistanceLevelListener(new RockerView.OnDistanceLevelListener() {
            @Override
            public void onDistanceLevel(int level) {
//                speedLevel = 225 / 9 * level;
//                switch (dir) {
//                    case 0:
//                        break;
//                    case 1:
//                        break;
//                    case 2:
//                        break;
//                    case 3:
//                        break;
//                    case 4:
//                        break;
//                    case 5:
//                        break;
//                    case 6:
//                        break;
//                    case 7:
//                        break;
//                    case 8:
//                        break;
//                }
            }
        });

    }

    @Override
    public void onPause()
    {
        super.onPause();
        try
        {
            //Don't leave Bluetooth sockets open when leaving activity
            btSocket.close();
        } catch (IOException e2) {
            //insert code to deal with this
        }
    }

    //create new class for connect thread
    private class ConnectedThread extends Thread {
        private final InputStream mmInStream;
        private final OutputStream mmOutStream;

        //creation of the connect thread
        public ConnectedThread(BluetoothSocket socket) {
            InputStream tmpIn = null;
            OutputStream tmpOut = null;

            try {
                //Create I/O streams for connection
                tmpIn = socket.getInputStream();
                tmpOut = socket.getOutputStream();
            } catch (IOException e) { }

            mmInStream = tmpIn;
            mmOutStream = tmpOut;
        }

        public void run() {
            int bytes;

            // Keep looping to listen for received messages
            while (true) {
                try {
                    byte[] buffer = new byte[512];
                    bytes = mmInStream.read(buffer);            //read bytes from input buffer
                    //String readMessage = new String(buffer, 0, bytes);
                    // Send the obtained bytes to the UI Activity via handler
                    //bluetoothIn.obtainMessage(Constants.MESSAGE_READ, bytes, -1, buffer).sendToTarget();
                    bluetoothIn.obtainMessage(handlerState, bytes, -1, buffer).sendToTarget();
                } catch (IOException e) {
                    break;
                }
            }
        }
        //write method
        public void write(String input) {
            byte[] msgBuffer = input.getBytes();           //converts entered String into bytes
            try {
                mmOutStream.write(msgBuffer);                //write bytes over BT connection via outstream
            } catch (IOException e) {
                //if you cannot write, close the application
                Toast.makeText(getBaseContext(), "Connection Failure", Toast.LENGTH_LONG).show();
                finish();

            }
        }
    }

    void drawCameraImage(){
        cameraLayout=(LinearLayout) findViewById(R.id.cameraView);
        cameraLayout.removeAllViews();
        cameraDraw=new DrawView(this);
        cameraDraw.setMinimumHeight(cameraLayout.getHeight());
        cameraDraw.setMinimumWidth(cameraLayout.getWidth());
        cameraDraw.invalidate();
        cameraLayout.addView(cameraDraw);
    }

    public class DrawView extends View{

        public DrawView(Context context){
            super(context);
            //TODO AUTO-generated constructor stub
        }
        @Override
        protected void onDraw(Canvas canvas){
            super.onDraw(canvas);

            Paint bluePaint=new Paint();
            Paint whitePaint=new Paint();
            bluePaint.setAntiAlias(true);
            bluePaint.setColor(Color.BLUE);
            whitePaint.setAntiAlias(true);
            whitePaint.setColor(Color.WHITE);

            byte CamByte;

            for(int row=0;row<camHeight;row++){
                for(int col=0;col<camWidth;col+=8){
                    CamByte=camArray[row*camWidth/8+col/8];
                    if((CamByte&0b10000000) >0){
                        cam2DArray[row][col]=1;
                        canvas.drawRect(col*5,row*5,col*5+5,row*5+5,bluePaint);
                    }else{
                        cam2DArray[row][col]=0;
                    }
                    if((CamByte&0b01000000) >0){
                        cam2DArray[row][col+1]=1;
                        canvas.drawRect((col+1)*5,row*5,(col+1)*5+5,row*5+5,bluePaint);
                    }else{
                        cam2DArray[row][col+1]=0;
                    }
                    if((CamByte&0b00100000) >0){
                        cam2DArray[row][col+2]=1;
                        canvas.drawRect((col+2)*5,row*5,(col+2)*5+5,row*5+5,bluePaint);
                    }else{
                        cam2DArray[row][col+2]=0;
                    }
                    if((CamByte&0b00010000) >0){
                        cam2DArray[row][col+3]=1;
                        canvas.drawRect((col+3)*5,row*5,(col+3)*5+5,row*5+5,bluePaint);
                    }else{
                        cam2DArray[row][col+3]=0;
                    }
                    if((CamByte&0b00001000) >0){
                        cam2DArray[row][col+4]=1;
                        canvas.drawRect((col+4)*5,row*5,(col+4)*5+5,row*5+5,bluePaint);
                    }else{
                        cam2DArray[row][col+4]=0;
                    }
                    if((CamByte&0b00000100) >0){
                        cam2DArray[row][col+5]=1;
                        canvas.drawRect((col+5)*5,row*5,(col+5)*5+5,row*5+5,bluePaint);
                    }else{
                        cam2DArray[row][col+5]=0;
                    }
                    if((CamByte&0b00000010) >0){
                        cam2DArray[row][col+6]=1;
                        canvas.drawRect((col+6)*5,row*5,(col+6)*5+5,row*5+5,bluePaint);
                    }else{
                        cam2DArray[row][col+6]=0;
                    }
                    if((CamByte&0b00000001) >0){
                        cam2DArray[row][col+7]=1;
                        canvas.drawRect((col+7)*5,row*5,(col+7)*5+5,row*5+5,bluePaint);
                    }else{
                        cam2DArray[row][col+7]=0;
                    }
                }
            }

//            for(int y=0;y<camHeight;y++){
//                for(int x=0;x<camWidth;x++){
//                    if(cam2DArray[y][x]==1){
//                        canvas.drawRect(x*5,y*5,x*5+5,y*5+5,bluePaint);
//                    }else if(cam2DArray[y][x]==0){
//                        canvas.drawRect(x*5,y*5,x*5+5,y*5+5,whitePaint);
//                    }else if(cam2DArray[y][x]==2){
//                    }else if(cam2DArray[y][x]==3){
//                    }
//                }
//            }
        }
    }

    void control(int dir){
        if (btSocket!=null)
        {
            String buffer=Character.toString((char)dir);
            switch (dir) {
                case 0:
                    direction.setText("暫停");
                    break;
                case 1:
                    direction.setText("右");
                    break;
                case 2:
                    direction.setText("右上");
                    break;
                case 3:
                    direction.setText("上");
                    break;
                case 4:
                    direction.setText("左上");
                    break;
                case 5:
                    direction.setText("左");
                    break;
                case 6:
                    direction.setText("左下");
                    break;
                case 7:
                    direction.setText("下");
                    break;
                case 8:
                    direction.setText("右下");
                    break;
            }
            buffer+=Character.toString((char)dir);
            try
            {
                btSocket.getOutputStream().write(buffer.getBytes());
            }
            catch (IOException e)
            {
            }
        }
    }
}