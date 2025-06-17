/* @file AnalysisDialog.java
 *
 * @author marco corvi
 * @date mar 2024
 *
 * @brief NasoPlus analysis dialog
 *
 * --------------------------------------------------------
 *  Copyright This software is distributed under GPL-3.0 or later
 *  See the file COPYING.
 * ----------------------------------------------------------
 */
package com.nasoplus;

import android.content.Context;
// import android.content.Intent;

import android.app.Dialog;
import android.app.DatePickerDialog;
import android.app.DatePickerDialog.OnDateSetListener;
import android.app.TimePickerDialog;
import android.app.TimePickerDialog.OnTimeSetListener;

import android.widget.Button;
import android.widget.TextView;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.ArrayAdapter;
import android.widget.AdapterView;

import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup.LayoutParams;
import android.widget.TimePicker;
// import android.widget.TimePicker.OnTimeChangedListener;
import android.widget.DatePicker;
// import android.widget.DatePicker.OnDateChangedListener;

// import android.util.Log;

// import android.net.Uri;

import java.util.List;

class AnalysisDialog extends Dialog
                     implements OnClickListener
                     , OnTimeSetListener
                     , OnDateSetListener
                     , AdapterView.OnItemSelectedListener
{
  private Context mContext;
  private NasoActivity mParent;
  private String mLogname;
  
  private TextView   mTVinterval;
  private TextView   mTVzerogas;
  private EditText   mETinGas;
  private EditText   mEToutArea;
  private EditText   mEToutTemp;
  private EditText   mEToutPressure;
  private EditText   mETmolarMass;
  private Button     mBTday;
  private Button     mBThour;

  private TextView  mTVq;
  private TextView  mTVv;

  final static float   R_GAS        = 8.31446f; // gas constant [J/mol g K]
  private static int   mInterval;
  private static int   mZeroGas;
  private static int   mInGas       = 0; // [g]
  private static float mOutArea     = 0; // [m2]
  private static float mOutTemp     = -273; // [C]
  private static int   mOutPressure = 0; // [mBar]
  private static float mMolarMass   = 0;   // [g]
  private static String mDatetime   = null; // input time [arduino format]
  private static float mMolarVolume = 0.023f;      // [m3] molar_volume = R T / P
                                    // J /(mol g K) * K / (100 Pa) = N m /(mol g) * 1/(100 N/m2) = 8.31446 * (273.15 + T[C]) / P[mBar]

  private static String[] GAS_TYPES = { "Molar mass", "methane", "ethane", "propane", "butane" };
  private static float[] MOLAR_MASS = { 0, 16.04f, 30.06f, 44.097f, 58.12f };

  AnalysisDialog( Context context, NasoActivity parent, String logname )
  {
    super( context );
    mContext = context;
    mParent  = parent;
    mLogname = logname;
    setContentView(R.layout.analysis_dialog);
    getWindow().setLayout( LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT );

    setTitle( logname );

    ((Button)findViewById(R.id.btn_cancel)).setOnClickListener( this );
    ((Button)findViewById(R.id.btn_compute)).setOnClickListener( this );

    mTVinterval = (TextView)findViewById( R.id.interval );
    mTVzerogas  = (TextView)findViewById( R.id.zerogas  );
    mETinGas    = (EditText)findViewById( R.id.ingas    );
    mEToutArea  = (EditText)findViewById( R.id.outarea  );
    mEToutTemp  = (EditText)findViewById( R.id.outtemp  );
    mEToutPressure  = (EditText)findViewById( R.id.outpressure  );
    mETmolarMass  = (EditText)findViewById( R.id.molarmass  );
    Spinner spinnerGas   = (Spinner)findViewById( R.id.gas_type );
    
    spinnerGas.setAdapter( new ArrayAdapter<>( mContext, R.layout.menu, GAS_TYPES ) );
    spinnerGas.setOnItemSelectedListener( this );

    mInterval = ( mParent.mInterval > 0 )? mParent.mInterval : 30;
    mZeroGas  = ( mParent.mZerogas  > 0 )? mParent.mZerogas  : 18;
    if ( mInGas == 0 ) mInGas = 500;
    if ( mOutArea == 0 ) mOutArea = 1.0f;
    if ( mOutTemp == -273 ) mOutTemp = 15;
    if ( mOutPressure == 0 ) mOutPressure = 1023; // sea level
    if ( mMolarMass == 0 ) mMolarMass = 44; // C3H8
    if ( mDatetime == null ) mDatetime = NasoUtil.currentDateTime(); // arduino format

    mBTday  = (Button)findViewById( R.id.inday );
    mBThour = (Button)findViewById( R.id.inhour );
    mBTday.setOnClickListener( this );
    mBThour.setOnClickListener( this );
    mBTday.setText( NasoUtil.getDate( mDatetime ) );  // yyyy-mm-dd [arduino format]
    mBThour.setText( NasoUtil.getTime( mDatetime ) ); // HH:MM

    mTVinterval.setText( Integer.toString(mInterval) );
    mTVzerogas.setText(  Integer.toString(mZeroGas) );
    mETinGas.setText(    Integer.toString(mInGas) );
    mEToutArea.setText(  String.format("%.2f", mOutArea) );
    mEToutTemp.setText(  String.format("%.1f", mOutTemp) );
    mEToutPressure.setText( Integer.toString(mOutPressure) );
    mETmolarMass.setText(  String.format("%.1f", mMolarMass) );

    mTVq = (TextView)findViewById( R.id.flux );
    mTVv = (TextView)findViewById( R.id.volume );
  }

  @Override
  public void onItemSelected( AdapterView av, View v, int pos, long id ) 
  { 
    if ( pos == 0 ) return;
    // get molar mass
    mMolarMass = MOLAR_MASS[pos];
    mETmolarMass.setText( String.format("%.2f", mMolarMass) );
  }

  @Override
  public void onNothingSelected( AdapterView av ) { }

  private int getIntValue( EditText et )
  {
    try {
      CharSequence cs = et.getText();
      if ( cs != null ) return Integer.parseInt( cs.toString() );
    } catch ( NumberFormatException e ) { }
    return -1;
  }

  private float getFloatValue( EditText et )
  {
    try {
      CharSequence cs = et.getText();
      if ( cs != null ) return Float.parseFloat( cs.toString() );
    } catch ( NumberFormatException e ) { }
    return -1.0f;
  }

  private String getStringValue( Button et )
  {
    CharSequence cs = et.getText();
    if ( cs != null ) return cs.toString();
    return null;
  }

  @Override
  public void onClick( View v ) 
  {
    if ( v.getId() == R.id.inday ) {
      String date = mBTday.getText().toString();
      int y = NasoUtil.dateParseYear( date );
      int m = NasoUtil.dateParseMonth( date ) - 1; // convert to android month
      int d = NasoUtil.dateParseDay( date );
      new DatePickerDialog( mContext, this, y, m, d ).show();
      return;
    } else if ( v.getId() == R.id.inhour ) {
      String time = mBTday.getText().toString();
      int h = NasoUtil.timeParseHour( time );
      int m = NasoUtil.timeParseMinute( time );
      new TimePickerDialog( mContext, this, h, m, true ).show(); // true 24-hour
      return;
    } else 
    if ( v.getId() == R.id.btn_compute ) {
      mInGas       = getIntValue( mETinGas   );
      mOutArea     = getFloatValue( mEToutArea );
      mOutTemp     = getFloatValue( mEToutTemp );
      mOutPressure = getIntValue( mEToutPressure );
      mMolarMass   = getFloatValue( mETmolarMass );
      mDatetime    = getStringValue( mBTday ) + " " + getStringValue( mBThour ) + ":00"; // arduino format
      NasoUtil.log( "values " + mInterval + " " + mZeroGas + " " + mInGas + " <" + mDatetime + ">" ); 
      doAnalysis( );
      return;
    }
    dismiss();
  }

  public void onDateSet( DatePicker view, int year, int month, int dayOfMonth)
  {
    mBTday.setText( String.format("%4d-%02d-%02d", year, month+1, dayOfMonth ) ); // arduino format
  }

  public void onTimeSet( TimePicker view, int hourOfDay, int minute)
  {
    mBThour.setText( String.format("%02d:%02d", hourOfDay, minute ) );
  }

  final static int ZERO_COUNT = 10;


  void doAnalysis()
  {
    List< LogData > datalog = mParent.getDataLog();
    if ( datalog == null ) {
      NasoUtil.toast( mContext, R.string.error_no_log );
      return;
    }
    int nr_data = datalog.size();
    NasoUtil.log( "analyze data log: nr " + nr_data );
    int n = 0;
    LogData data = datalog.get( n );
    float zero_adc = data.adc;
    for ( ; n < nr_data; ++n ) {
      data = datalog.get( n );
      if ( data.adc < zero_adc ) zero_adc = data.adc;
    }
    zero_adc *= 1.5f;
 
    int n_start = 0;
    int zero_cnt = 0;
    for ( ; n_start < nr_data; ++n_start ) {
      data = datalog.get( n_start );
      if ( data.adc <= zero_adc ) {
        if ( zero_cnt ++ > ZERO_COUNT ) break;
      } else {
        zero_cnt = 0;
      }
    }
    NasoUtil.log( "analyze data log: start " + n_start );
    if ( n_start +1 >= nr_data ) {
      NasoUtil.toast( mContext, R.string.error_bad_log );
      return;
    }
    // compute interval
    LogData data1 = datalog.get( n_start );
    LogData data2 = datalog.get( n_start+1 );
    String start_time  = NasoUtil.arduinoToAndroidDate( data1.datetime );
    String start_time2 = NasoUtil.arduinoToAndroidDate( data2.datetime );
    int interval = NasoUtil.secondsDifference( start_time, start_time2 );
    NasoUtil.log( "analyze data log: interval " + interval );
    NasoUtil.log( "analyze data log: start-time <" + start_time + ">" );

    // compute delay of first data;
    int delay = 0;
    if ( mDatetime != null ) {
      String in_time = NasoUtil.arduinoToAndroidDate( mDatetime );
      delay = NasoUtil.secondsDifference( in_time, start_time );
      NasoUtil.log( "analyze data log: delay " + delay );
    }

    float R0 = 1023.0f / zero_adc - 1.0f;

    float Qadc = 0;
    float Vadc = 0;
//    float Qppm = 0;
//    float Vppm = 0;
    if ( interval > 0 && mInGas > 0 ) {
      mMolarVolume = R_GAS * ( 273.15f + mOutTemp ) / ( mOutPressure * 100 ); // mBar = hPa
      NasoUtil.log( " molar volume " + mMolarVolume );
      float M0adc = 0;
      float M1adc = 0;
//      float M0ppm = 0;
//      float M1ppm = 0;
      int cnt = 0;
      for ( n = n_start; n < nr_data; ++n ) {
        ++ cnt;
        data = datalog.get( n );
        // TODO corrections
        float R = ( 1023.0f / data.adc - 1.0f ) / R0;
        float logR = (float)( Math.log10( R ) );
        float adc = (float)( Math.pow( 10.0, 0.640 - 2.154 * logR ) );
        M0adc += adc;
        M1adc += adc * delay;
//        float ppm = (float)(1.7828 * Math.pow( data.ppm, 0.434 ) );
//        M0ppm += ppm;
//        M1ppm += ppm * delay;
        delay += interval;
      }
      NasoUtil.log( "ADC M0 " + M0adc + " M1 " + M1adc );
      M0adc *= interval;
      M1adc *= interval;
//      M0ppm *= interval;
//      M1ppm *= interval;

      Qadc = (mMolarVolume / mMolarMass) * 1000000 * mInGas / M0adc * mOutArea; // m3/s
      Vadc = Qadc * M1adc / M0adc;



//      Qppm = (mMolarVolume / mMolarMass) * 1000000 * mInGas / M0ppm * mOutArea; // m3/s
//      Vppm = Qppm * M1ppm / M0ppm;
      NasoUtil.log( "Q " + Qadc + " V " + Vadc );
      mTVq.setText( String.format( "%.2f m3/s", Qadc ) );
      mTVv.setText( String.format( "%.0f m3", Vadc ) ); 
      mTVinterval.setText(  String.format( "%d", interval ) );
      mTVzerogas.setText( String.format( "%.0f adc-V", zero_adc ) ); 
    }
  }

}
