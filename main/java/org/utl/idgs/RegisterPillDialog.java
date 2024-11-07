// RegisterPillDialog.java
package org.utl.idgs;

import android.app.Dialog;
import android.app.TimePickerDialog;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TimePicker;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.DialogFragment;

import java.util.Calendar;
import java.util.Locale;

public class RegisterPillDialog extends DialogFragment {

    private EditText editTextPillName;
    private EditText editTextPillTime;
    private Button buttonRegister;

    public interface RegisterPillDialogListener {
        void onPillRegistered(String pillName, String pillTime);
    }

    @NonNull
    @Override
    public Dialog onCreateDialog(@Nullable Bundle savedInstanceState) {
        View view = LayoutInflater.from(getActivity()).inflate(R.layout.dialog_register_pill, null);

        editTextPillName = view.findViewById(R.id.editTextPillName);
        editTextPillTime = view.findViewById(R.id.editTextPillTime);
        buttonRegister = view.findViewById(R.id.buttonRegister);

        editTextPillTime.setOnClickListener(v -> openTimePicker());

        buttonRegister.setOnClickListener(v -> {
            String pillName = editTextPillName.getText().toString();
            String pillTime = editTextPillTime.getText().toString();

            if (TextUtils.isEmpty(pillName) || TextUtils.isEmpty(pillTime)) {
                Toast.makeText(getActivity(), "Por favor ingrese nombre y hora", Toast.LENGTH_SHORT).show();
            } else {
                ((RegisterPillDialogListener) getActivity()).onPillRegistered(pillName, pillTime);
                dismiss();
            }
        });

        Dialog dialog = new Dialog(getActivity());
        dialog.setContentView(view);
        return dialog;
    }

    private void openTimePicker() {
        Calendar calendar = Calendar.getInstance();
        int hour = calendar.get(Calendar.HOUR_OF_DAY);
        int minute = calendar.get(Calendar.MINUTE);

        new TimePickerDialog(getActivity(), (TimePicker view, int selectedHour, int selectedMinute) -> {
            String formattedTime = String.format(Locale.getDefault(), "%02d:%02d", selectedHour, selectedMinute);
            editTextPillTime.setText(formattedTime);
        }, hour, minute, true).show();
    }
}
