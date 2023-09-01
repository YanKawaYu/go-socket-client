package com.kawayu.go_socket_android

import android.os.Bundle
import android.util.Log
import android.widget.Toast
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import com.kawayu.go_socket_android.ui.theme.GosocketandroidTheme

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            GosocketandroidTheme {
                // A surface container using the 'background' color from the theme
                Surface(
                    modifier = Modifier.fillMaxSize(),
                    color = MaterialTheme.colorScheme.background
                ) {
                    Greeting("Android")
                    TextButton(onClick = {
                        if (TcpClient.getConnectStatus() == TcpClient.CONN_CONNECTED) {
                            TcpClient.send("chat.AddMessage", "{\"message\":\"This is a message\"}", null)
                            { _: Int, response: String ->
                                Log.i("gotcp", "Content from server: $response")
                            }
                        }else {
                            Toast.makeText(this, "Failed to connect to the server", Toast.LENGTH_SHORT).show()
                        }
                    }) {
                        Text(text = "Send message")
                    }
                }
            }
        }
    }

    override fun onStart() {
        super.onStart()
        TcpClient.initHostAndPort("192.168.0.191", "192.168.0.191", 8080, false) {
            val payload = "{\"username\":\"haha\", \"password\":\"xxxxx\"}"
            payload
        }
        TcpClient.makeSureConnected()
    }
}

@Composable
fun Greeting(name: String, modifier: Modifier = Modifier) {
    Text(
        text = "Hello $name!",
        modifier = modifier
    )
}

@Preview(showBackground = true)
@Composable
fun GreetingPreview() {
    GosocketandroidTheme {
        Greeting("Android")
    }
}