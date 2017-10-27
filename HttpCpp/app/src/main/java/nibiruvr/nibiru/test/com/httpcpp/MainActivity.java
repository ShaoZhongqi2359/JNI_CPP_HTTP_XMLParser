package nibiruvr.nibiru.test.com.httpcpp;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

import java.util.HashMap;
import java.util.Map;

public class MainActivity extends Activity {

    private Button mButton;
    private Button mXmlButton;

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("nibiruhttp");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        mButton = (Button) findViewById(R.id.btnTest);
        mXmlButton = findViewById(R.id.btnXmlTest);

        mButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                new Thread() {
                    @Override
                    public void run() {
                        super.run();
                        //getHttpInfo("");
                        String URL = "http://rom.1919game.net:8081/NibiruRom/app/CheckVRKeyMapAction?keymapTime=1502444523662&channel=VR0829&romVersion=3&version=1";
                        String PATH = "/sdcard/shao2.xml";
                        int value = requestXmlInfo(URL,PATH);
                        if(value == 200){
                            HashMap<String, Config> map = getConfigMapValue(PATH);
                            map.size();
                        }else {

                        }
                    }
                }.start();
            }
        });

        mXmlButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                HashMap<String, Config> map = getConfigMapValue("");
                for (Map.Entry<String, Config> entry : map.entrySet()) {
                    String name = entry.getKey();
                    Config test = entry.getValue();
                }

            }
        });

    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    public native void getHttpInfo(String url);

    public native void parseXml();

    public native int requestXmlInfo(String URL,String filePath);

    public native HashMap<String, Config> getConfigMapValue(String filePath);



}
