package org.utl.idgs;

import android.annotation.SuppressLint;
import android.app.DatePickerDialog;
import android.app.TimePickerDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.Spinner;
import android.widget.Toast;

import androidx.activity.EdgeToEdge;
import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Locale;
import java.util.Set;
import java.util.UUID;

public class MainActivity extends AppCompatActivity implements RegisterPillDialog.RegisterPillDialogListener {

    private static final int REQUEST_ENABLE_BT = 1;
    private static final int REQUEST_PERMISSION_BT = 2;
    private EditText txtHora, txtFecha;
    private ImageButton btnHora, btnFecha, btnRegister;
    private Spinner slcDevices;
    private BluetoothAdapter bluetoothAdapter;
    private BluetoothSocket bluetoothSocket;
    private OutputStream outputStream;
    private BluetoothDevice selectedDevice;
    private static final String UUID_BT = "00001101-0000-1000-8000-00805f9b34fb";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_main);

        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });

        initializeComponents();
        setupBluetooth();

        // Inicializar la hora y la fecha por defecto
        setDefaultDateTime();
    }

    private void initializeComponents() {
        txtHora = findViewById(R.id.txtHora);
        txtFecha = findViewById(R.id.txtFecha);
        btnHora = findViewById(R.id.btnHora);
        btnFecha = findViewById(R.id.btnFecha);
        btnRegister = findViewById(R.id.btnRegister);
        slcDevices = findViewById(R.id.slcDevices);

        btnHora.setOnClickListener(v -> openTimePicker());
        btnFecha.setOnClickListener(v -> openDatePicker());
        btnRegister.setOnClickListener(v -> openRegisterPillDialog());
    }

    private void openRegisterPillDialog() {
        RegisterPillDialog dialog = new RegisterPillDialog();
        dialog.show(getSupportFragmentManager(), "RegisterPillDialog");
    }

    private void setDefaultDateTime() {
        Calendar calendar = Calendar.getInstance();
        String formattedTime = getFormattedTime(calendar);
        String formattedDate = getFormattedDate(calendar);

        txtHora.setText(formattedTime);
        txtFecha.setText(formattedDate);
    }

    private String getFormattedTime(Calendar calendar) {
        int hour = calendar.get(Calendar.HOUR) == 0 ? 12 : calendar.get(Calendar.HOUR);
        int minute = calendar.get(Calendar.MINUTE);
        String amPm = (calendar.get(Calendar.AM_PM) == Calendar.AM) ? "AM" : "PM";
        return String.format(Locale.getDefault(), "%02d:%02d %s", hour, minute, amPm);
    }

    private String getFormattedDate(Calendar calendar) {
        int year = calendar.get(Calendar.YEAR);
        int month = calendar.get(Calendar.MONTH) + 1; // 0-indexed
        int day = calendar.get(Calendar.DAY_OF_MONTH);
        return String.format(Locale.getDefault(), "%02d/%02d/%d", day, month, year);
    }

    private void setupBluetooth() {
        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (bluetoothAdapter == null) {
            showToast("Bluetooth no es soportado");
            finish();
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            setupDeviceList();
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.S)
    private void setupDeviceList() {
        if (!bluetoothAdapter.isEnabled()) {
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            if (ActivityCompat.checkSelfPermission(this, android.Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
                // TODO: Consider calling
                //    ActivityCompat#requestPermissions
                // here to request the missing permissions, and then overriding
                //   public void onRequestPermissionsResult(int requestCode, String[] permissions,
                //                                          int[] grantResults)
                // to handle the case where the user grants the permission. See the documentation
                // for ActivityCompat#requestPermissions for more details.
                return;
            }
            startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
        }

        if (!hasBluetoothPermissions()) {
            requestBluetoothPermissions();
            return;
        }

        populatePairedDevices();
    }

    private boolean hasBluetoothPermissions() {
        return ActivityCompat.checkSelfPermission(this, android.Manifest.permission.BLUETOOTH_CONNECT) == PackageManager.PERMISSION_GRANTED &&
                ActivityCompat.checkSelfPermission(this, android.Manifest.permission.BLUETOOTH_SCAN) == PackageManager.PERMISSION_GRANTED;
    }

    private void requestBluetoothPermissions() {
        ActivityCompat.requestPermissions(this,
                new String[]{android.Manifest.permission.BLUETOOTH_CONNECT, android.Manifest.permission.BLUETOOTH_SCAN},
                REQUEST_PERMISSION_BT);
    }

    private void populatePairedDevices() {
        if (ActivityCompat.checkSelfPermission(this, android.Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
            // TODO: Consider calling
            //    ActivityCompat#requestPermissions
            // here to request the missing permissions, and then overriding
            //   public void onRequestPermissionsResult(int requestCode, String[] permissions,
            //                                          int[] grantResults)
            // to handle the case where the user grants the permission. See the documentation
            // for ActivityCompat#requestPermissions for more details.
            return;
        }
        Set<BluetoothDevice> pairedDevices = bluetoothAdapter.getBondedDevices();
        ArrayList<BluetoothDevice> devicesList = new ArrayList<>(pairedDevices);
        String[] deviceNames = new String[devicesList.size() + 1];
        deviceNames[0] = "----";

        int index = 1;
        for (BluetoothDevice device : pairedDevices) {
            deviceNames[index++] = device.getName();
        }

        ArrayAdapter<String> adapter = new ArrayAdapter<>(this, android.R.layout.simple_spinner_item, deviceNames);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        slcDevices.setAdapter(adapter);

        slcDevices.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                selectedDevice = position == 0 ? null : devicesList.get(position - 1);
                if (selectedDevice != null) connectToSelectedDevice();
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
                selectedDevice = null;
            }
        });
    }

    private void connectToSelectedDevice() {
        if (selectedDevice == null) {
            showToast("No se ha seleccionado un dispositivo");
            return;
        }

        new Thread(() -> {
            try {
                if (!hasBluetoothPermissions()) {
                    showToast("Permiso Bluetooth no otorgado");
                    return;
                }

                if (ActivityCompat.checkSelfPermission(this, android.Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
                    // TODO: Consider calling
                    //    ActivityCompat#requestPermissions
                    // here to request the missing permissions, and then overriding
                    //   public void onRequestPermissionsResult(int requestCode, String[] permissions,
                    //                                          int[] grantResults)
                    // to handle the case where the user grants the permission. See the documentation
                    // for ActivityCompat#requestPermissions for more details.
                    return;
                }
                bluetoothSocket = selectedDevice.createRfcommSocketToServiceRecord(UUID.fromString(UUID_BT));
                if (ActivityCompat.checkSelfPermission(this, android.Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
                    // TODO: Consider calling
                    //    ActivityCompat#requestPermissions
                    // here to request the missing permissions, and then overriding
                    //   public void onRequestPermissionsResult(int requestCode, String[] permissions,
                    //                                          int[] grantResults)
                    // to handle the case where the user grants the permission. See the documentation
                    // for ActivityCompat#requestPermissions for more details.
                    return;
                }
                bluetoothSocket.connect();
                outputStream = bluetoothSocket.getOutputStream();
                showToast("Conexión Bluetooth establecida con " + selectedDevice.getName());

            } catch (IOException e) {
                showToast("Error al conectar con " + selectedDevice.getName());
                Log.e("Bluetooth", "Error de conexión", e);
                closeSocket();
            }
        }).start();
    }

    private void closeSocket() {
        if (bluetoothSocket != null) {
            try {
                bluetoothSocket.close();
            } catch (IOException e) {
                Log.e("Bluetooth", "Error al cerrar el socket", e);
            }
        }
    }

    private void openTimePicker() {
        Calendar calendar = Calendar.getInstance();
        int hour = calendar.get(Calendar.HOUR);
        int minute = calendar.get(Calendar.MINUTE);

        TimePickerDialog timePicker = new TimePickerDialog(this, (view, selectedHour, selectedMinute) -> {
            String formattedTime = getFormattedTime(calendar);
            txtHora.setText(formattedTime);
            sendBluetoothMessage("/setHora " + formattedTime);
        }, hour, minute, false);

        timePicker.show();
    }

    private void openDatePicker() {
        Calendar calendar = Calendar.getInstance();
        int year = calendar.get(Calendar.YEAR);
        int month = calendar.get(Calendar.MONTH);
        int day = calendar.get(Calendar.DAY_OF_MONTH);

        DatePickerDialog datePicker = new DatePickerDialog(this, (view, selectedYear, selectedMonth, selectedDay) -> {
            String formattedDate = String.format(Locale.getDefault(), "%02d/%02d/%d", selectedDay, selectedMonth + 1, selectedYear);
            txtFecha.setText(formattedDate);
            sendBluetoothMessage("/setFecha " + formattedDate);
        }, year, month, day);

        datePicker.show();
    }

    private void sendBluetoothMessage(String message) {
        if (outputStream == null) {
            showToast("No hay conexión Bluetooth");
            return;
        }

        new Thread(() -> {
            try {
                outputStream.write(message.getBytes());
                showToast("Mensaje enviado: " + message);
            } catch (IOException e) {
                Log.e("Bluetooth", "Error al enviar mensaje", e);
                showToast("Error al enviar mensaje");
            }
        }).start();
    }

    private void showToast(String message) {
        runOnUiThread(() -> Toast.makeText(MainActivity.this, message, Toast.LENGTH_SHORT).show());
    }

    @Override
    public void onPillRegistered(String pillName, String pillTime) {
        // Construye el mensaje en el formato que el ESP32 espera
        String comando = "/registrar[" + pillName + "," + pillTime + "]";

        // Envía el comando a través de Bluetooth al ESP32
        sendBluetoothMessage(comando);

        // Muestra un mensaje de confirmación en la aplicación
        showToast("Pastilla registrada: " + pillName + " a las " + pillTime);
    }

    @Override
    public void onPointerCaptureChanged(boolean hasCapture) {
        super.onPointerCaptureChanged(hasCapture);
    }
}
