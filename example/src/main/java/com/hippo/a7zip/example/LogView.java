/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.hippo.a7zip.example;

import android.app.Activity;
import android.content.Context;
import android.graphics.Color;
import android.text.Spannable;
import android.text.SpannableString;
import android.text.TextUtils;
import android.text.style.ForegroundColorSpan;
import android.util.*;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ScrollView;
import android.widget.TextView;

/** Simple TextView which is used to output log data received through the LogNode interface.
 */
public class LogView extends ScrollView {

    private static final String TAG = "a7zip";

    private TextView mTextView;

    public LogView(Context context) {
        super(context);
        init(context);
    }

    public LogView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init(context);
    }

    public LogView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        init(context);
    }

    private void init(Context context) {
        mTextView = new TextView(context);
        addView(mTextView, ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT);
    }

    public void println(String msg) {
        println(android.util.Log.DEBUG, msg, null);
    }

    /**
     * Formats the log data and prints it out to the LogView.
     * @param priority Log level of the data being logged.  Verbose, Error, etc.
     * @param msg The actual message to be logged. The actual message to be logged.
     * @param tr If an exception was thrown, this can be sent along for the logging facilities
     *           to extract and print useful information.
     */
    public void println(int priority, String msg, Throwable tr) {
        if (TextUtils.isEmpty(msg) && tr == null) {
            return;
        }

        int priorityColor = Color.BLACK;

        // For the purposes of this View, we want to print the priority as readable text.
        switch(priority) {
            case android.util.Log.VERBOSE:
                priorityColor = 0xde000000;
                break;
            case android.util.Log.DEBUG:
                priorityColor = 0xff4caf50;
                break;
            case android.util.Log.INFO:
                priorityColor = 0xff2196f3;
                break;
            case android.util.Log.WARN:
                priorityColor = 0xffffc107;
                break;
            case android.util.Log.ERROR:
                priorityColor = 0xfff44336;
                break;
            case android.util.Log.ASSERT:
                priorityColor = 0xffe91e63;
                break;
            default:
                break;
        }

        // Handily, the Log class has a facility for converting a stack trace into a usable string.
        String exceptionStr = null;
        if (tr != null) {
            exceptionStr = android.util.Log.getStackTraceString(tr);
        }

        // Take the priority, tag, message, and exception, and concatenate as necessary
        // into one usable line of text.
        StringBuilder outputBuilder = new StringBuilder();

        appendIfNotNull(outputBuilder, msg);
        appendIfNotNull(outputBuilder, exceptionStr);

        final SpannableString string = new SpannableString(outputBuilder.toString());
        string.setSpan(new ForegroundColorSpan(priorityColor), 0, string.length(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);

        // In case this was originally called from an AsyncTask or some other off-UI thread,
        // make sure the update occurs within the UI thread.
        ((Activity) getContext()).runOnUiThread((new Runnable() {
            @Override
            public void run() {
                // Display the text we just generated within the LogView.
                mTextView.append(string);
                post(new Runnable() {
                    @Override
                    public void run() {
                        fullScroll(View.FOCUS_DOWN);
                    }
                });
            }
        }));

        // logcat
        String logMsg;
        if (tr != null) {
            logMsg = msg + '\n' + Log.getStackTraceString(tr);
        } else {
            logMsg = msg;
        }
        Log.println(priority, TAG, logMsg);
    }

    /** Takes a string and adds to it, with a separator, if the bit to be added isn't null. Since
     * the logger takes so many arguments that might be null, this method helps cut out some of the
     * agonizing tedium of writing the same 3 lines over and over.
     * @param source StringBuilder containing the text to append to.
     * @param addStr The String to append
     * @return The fully concatenated String as a StringBuilder
     */
    private StringBuilder appendIfNotNull(StringBuilder source, String addStr) {
        if (addStr != null) {
            source.append(addStr).append("\n");
        }
        return source;
    }
}
