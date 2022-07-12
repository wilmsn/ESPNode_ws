var htmlversion = "3.xx";
var msg1_active = 0;
var msg2_active = 0;
var sw1_active = 0;
var sw2_active = 0;
var sw3_active = 0;
var sw4_active = 0;
var sw5_active = 0;
var rf24_active = 0;
var num_sw = 0;
var ledpwm = 0;
var red = 0;
var green = 0;
var blue = 0;
var ledmatrix_active = 0;
var neopixel_active = 0;
var ledpwm_active = 0;
var intervalperiode = 5000;

$(document).ready(function(){
  $("#titel2").hide();
  $("#msg1").hide();
  $("#msg2").hide();
  $("#sw1").hide();
  $("#sw2").hide();
  $("#sw3").hide();
  $("#sw4").hide();
  $("#sw5").hide();
  $("#sw1txt").hide();
  $("#sw2txt").hide();
  $("#sw3txt").hide();
  $("#sw4txt").hide();
  $("#ledmatrix").hide();
  if ( page == "settings" ) {
    $("#div_rf24").hide();
  }
  $.getJSON( "/cmd?webcfg1", function( data ) {
    $.each( data, function( key, val ) {
      prozessJS( key, val );
    });
    if ( page == "index" ) {
      var swwidth;
      var sw5width;
      switch (num_sw) {
        case 1:
          swwidth = "100%";
        break;  
        case 2:
          swwidth = "50%";
          sw5width = "40%";
        break;  
        case 3:
          swwidth = "33%";
        break;  
        case 4:
          swwidth = "25%";
        break;  
      }
      $("#sw1").css("width",swwidth);
      $("#sw2").css("width",swwidth);
      $("#sw3").css("width",swwidth);
      $("#sw4").css("width",swwidth);
      $("#sw5").css("width",sw5width);
      if ( ledmatrix_active ) {
        for (let y = 0; y < 24; y++) {
          $("#display").append("<div id='disp_line_"+y+"' class='ledmatrix_line'><div>");
            for (let x = 0; x < 32; x++) {
              var z = 32 * y + x;
              $("#disp_line_"+y).append("<div id='disp_dot_"+z+"' class='ledmatrix_dot'><div>");
            }
          }
      }
    }
  });
  $.getJSON( "/cmd?webcfg2", function( data ) {
    $.each( data, function( key, val ) {
      prozessJS( key, val );
    });
  });
  $.getJSON('/cmd?message1', function(data) {
    $.each( data, function( key, val ) {
      prozessJS( key, val );
    });
  });
  if ( page == "sysinfo" ) {
    $.getJSON( "/cmd?sysinfo1", function( data ) {
      $.each( data, function( key, val ) {
        prozessJS( key, val );
      });
    });
    $.getJSON( "/cmd?sysinfo2", function( data ) {
      $.each( data, function( key, val ) {
        prozessJS( key, val );
      });    
    });
    $.getJSON( "/cmd?sysinfo3", function( data ) {
      $.each( data, function( key, val ) {
        prozessJS( key, val );
      });
    });
    $.getJSON( "/cmd?sysinfo4", function( data ) {
      $.each( data, function( key, val ) {
        prozessJS( key, val );
      });
    });
    $.getJSON( "/cmd?sysinfo5", function( data ) {
      $.each( data, function( key, val ) {
        prozessJS( key, val );
      });
    });
    $.getJSON( "/cmd?sysinfo6", function( data ) {
      $.each( data, function( key, val ) {
        prozessJS( key, val );
      });
    });
    $("#f_htmlversion").html(htmlversion);
    $.getJSON( "/cmd?sensor1", function( data ) {
      $("#Tab>tbody").append("<tr><td colspan=2 class=\"tab_head\">Sensor:</td></tr> ");
      $.each( data, function( key, val ) {
        switch(key) {
          case "Sensor":
            $("#Tab>tbody").append("<tr><td class=\"tab_left\"> Sensor: </td><td class=\"tab_right\"> "+val+" </td></tr>");
          break;
          case "Resolution":
            $("#Tab>tbody").append("<tr><td class=\"tab_left\"> Resolution: </td><td class=\"tab_right\"> "+val+" </td></tr>");
          break;
          case "Temperatur":
            $("#Tab>tbody").append("<tr><td class=\"tab_left\"> Temperatur: </td><td class=\"tab_right\"> "+val+" &deg;C</td></tr>");
          break;
          case "Messwert":
            $("#Tab>tbody").append("<tr><td class=\"tab_left\"> Messwert: </td><td class=\"tab_right\"> "+val+" </td></tr>");
          break;
        }
      });
    });
    $.getJSON( "/cmd?rf24gw", function( data ) {
      $.each( data, function( key, val ) {
        switch(key) {
          case "gwno":
            $("#Tab>tbody").append("<tr><td colspan=2 class=\"tab_head\">RF24 Gateway:</td></tr> ");
            $("#Tab>tbody").append("<tr><td class=\"tab_left\"> Gateway Nummer: </td><td class=\"tab_right\"> "+val+" </td></tr>");
          break;
        }
      });
    });
  }
  if ( page == "settings" ) {
    $.getJSON( "/cmd?webcfg2", function( data ) {
      $.each( data, function( key, val ) {
        prozessJS( key, val );
      });
    });
  }
  if ( page == "index" ) {
    $.getJSON( "/cmd?status", function( data ) {
      $.each( data, function( key, val ) {
        prozessJS( key, val );
      });
    });
  }
  intervalfunction();
  setInterval(intervalfunction, intervalperiode);
});

function prozessJS( key, val ) {
      switch(key) {
        case "titel1":
          $("#titel1").html(val);
          $("#titel").html(val+" - "+title);
        break;
        case "titel2":
          $("#titel2").html(val);
          $("#titel2").show();
        break;
        case "msg1":
          if ( val == 1 ) {
            msg1_active = 1;
            $("#msg1").html("<p id='msg1txt'></p><hr>");
            $("#msg1").show();
          }
        break;
        case "msg2":
          if ( val == 1 ) {
            msg2_active = 1;
            $("#msg2").html("<p id='msg2txt'></p><hr>");
            $("#msg2").show();
          }
        break;
        case "msg1txt":
          $("#msg1txt").html(val);
        break;
        case "display":
          do_display(val);
        break;
        case "intensity":
          $("#slider1label").html("Helligkeit:"+val);
          $("#slider1").val(val);
        break;
        case "sw1":
          if ( val == 1 ) {
            sw1_active = 1;
            num_sw++;
            $("#sw1").show();
            do_sw1('state');
          }
        break;
        case "sw2":
          if ( val == 1 ) {
            num_sw++;
            sw2_active = 1;
            $("#sw2").show();
            do_sw2('state');
          }
        break;
        case "sw3":
          if ( val == 1 ) {
            num_sw++;
            sw3_active = 1;
            $("#sw3").show();
            do_sw3('state');
          }
        break;
        case "sw4":
          if ( val == 1 ) {
            num_sw++;
            sw4_active = 1;
            $("#sw4").show();
            do_sw4('state');
          }
        break;
        case "rf24":
          if ( val == 1 ) {
            rf24_active = 1;
            if ( page == "settings" ) {
              $("#div_rf24").show();
            }
          }
        break;
        case "ledmatrix":
          if ( val == 1 ) {
            num_sw++;
            ledmatrix_active = 1;
            sw5_active = 1;
            fill_ledmatrix();
            $("#ledmatrix").show();
            $("#sw5").show();
            $("#sw5cont").html("<label for='slider1' id='slider1label'></label><input type='range' name='slider1' id='slider1' min='0' max='15' value='0'>");	
            $.getJSON( "/cmd?status", function( data ) {
              $.each( data, function( key, val ) {
                prozessJS( key, val );
              });
            });
            intervalperiode = 30000;
            $("#slider1").change(function() {
               var wert = $(this).val(); 
               $("#slider1label").html("Helligkeit: "+wert);
               $.get( "/cmd?intensity="+wert, function( data ) {
               });
            });
          }
        break;
        case "ledpwm":
          if ( val == 1 ) {
            num_sw++;
            ledpwm_active = 1;
            sw5_active = 1;
            $("#sw5").show();
            $("#sw5cont").html("<label for='slider1' id='slider1label'></label><input type='range' name='slider1' id='slider1' min='0' max='255' value='0'>");	
            get_ledpwm();
            do_sw1("state");
            $("#slider1").change(function() {
               ledpwm = parseInt($(this).val());
               set_ledpwm();
            });
          }
        break;  
        case "neopixel":
          if ( val == 1 ) {
            num_sw++;
            neopixel_active = 1;
            sw5_active = 1;
            $("#sw5").show().css("height","160px").css("padding-bottom","unset").css("padding-top","unset").css("font-size","smaller");
            $("#sw5cont").html("<label for='slider1' id='slider1label'></label><input type='range' name='slider1' id='slider1' min='0' max='255' value='0'><label for='slider2' id='slider2label'></label><input type='range' name='slider2' id='slider2' min='0' max='255' value='0'><label for='slider3' id='slider3label'></label><input type='range' name='slider3' id='slider3' min='0' max='255' value='0'>");	
            $("#sw5txt").hide();
            get_rgb();
            do_sw1("state");
            $("#slider1").change(function() {
               red = parseInt($(this).val());
               set_rgb();
            });
            $("#slider2").change(function() {
               green = parseInt($(this).val()); 
               set_rgb();
            });
            $("#slider3").change(function() {
               blue = parseInt($(this).val()); 
               set_rgb();
            });
          }
        break;  
        case "IP":
          $("#f_localip").html(val);
        break;
        case "GW-IP":
          $("#f_gatewayip").html(val);
        break;
        case "DnsIP":
          $("#f_dnsip").html(val);
        break;
        case "SSID":
          $("#f_ssid").html(val);
        break;
        case "Channel":
          $("#f_channel").html(val);
        break;
        case "SubNetMask":
          $("#f_subnetmask").html(val);
        break;
        case "BSSID":
          $("#f_bssid").html(val);
        break;
        case "MAC":
          $("#f_macaddress").html(val);
        break;
        case "Hostname":
          $("#f_hostname").html(val);
        break;
        case "ResetReason":
          $("#f_resetreason").html(val);
        break;
        case "CpuFreq":
          $("#f_cpufreq").html(val);
        break;
        case "Freespace":
          $("#f_freespace").html(val);
        break;
        case "Sketchsize":
          $("#f_sketchsize").html(val);
        break;
        case "Heap_max":
          $("#f_maxheep").html(val);
        break;
        case "Heap_free":
          $("#f_freeheep").html(val);
        break;
        case "Heap_frag":
          $("#f_heapfrag").html(val+"%");
        break;
        case "FlashSize":
          $("#f_flashsize").html(val);
        break;
        case "FlashFreq":
          $("#f_flashspeed").html(val);
        break;
        case "CoreVer":
          $("#f_coreversion").html(val);
        break;
        case "IdeVer":
          $("#f_ideversion").html(val);
        break;
        case "MQTT-Hostname":
          $("#f_mqtthostname").html(val);
        break;
        case "MQTT-Server":
          $("#f_mqttserver").html(val);
        break;
        case "RF24HUB-Server":
          $("#f_rf24hubserver").html(val);
        break;
        case "RF24HUB-Port":
          $("#f_rf24hubport").html(val);
        break;
        case "RF24GW-Port":
          $("#f_rf24gwport").html(val);
        break;
        case "SdkVer":
          $("#f_sdkversion").html(val);
        break;
        case "SW":
          $("#f_swversion").html(val);
        break;
        case "UpTime":
          $("#f_uptime").html(val);
        break;
        case "logfile":
          if ( val == 1 ) $("#logfile").attr('checked', true);
        break;
        case "logger":
          if ( val == 1 ) $("#console").attr('checked', true);
        break;
        case "log_sensor":
          if ( val == 1 ) $("#log_sensor").attr('checked', true);
        break;
        case "log_rf24":
          if ( val == 1 ) $("#log_rf24").attr('checked', true);
        break;
        case "log_startup":
          if ( val == 1 ) $("#log_startup").attr('checked', true);
        break;
        case "log_mqtt":
          if ( val == 1 ) $("#log_mqtt").attr('checked', true);
        break;
        case "log_webcmd":
          if ( val == 1 ) $("#log_webcmd").attr('checked', true);
        break;
        case "log_sysinfo":
          if ( val == 1 ) $("#log_sysinfo").attr('checked', true);
        break;
      }
}

function fill_ledmatrix() {
    if ( ((sw1_active ==1) && ($("#sw1sw").html() == "Ein")) || (sw1_active == 0) ) {
      $.get("/cmd?matrixFB", function(data) {
      //alert(data[9]);
        for (let i = 0; i < data.length; i++) {
          if ( data[i] == "1" ) {
            $("#disp_dot_"+i).css("background","white");
          } else {
            $("#disp_dot_"+i).css("background","gray");
          }
        }
      });
    } else {
      for (let i = 0; i < 768; i++) {
        $("#disp_dot_"+i).css("background","gray");
      }
    }
}

function do_display( val ) {
  //alert("display");  
  $("#sw1sw").html(val);
  if (val == "Ein") {
    $("#sw1sw").removeClass("bg_bk").addClass("bg_yl");
  } else {
    $("#sw1sw").removeClass("bg_yl").addClass("bg_bk");
  }
}

function do_sw1( cmd ) {
  $.getJSON( "/cmd?sw1="+cmd, function( data ) {
    $.each( data, function( key, val ) {
      switch(key) {
      case "state":
        $("#sw1sw").html(val);
        if (val == "Ein") {
          $("#sw1sw").removeClass("bg_bk").addClass("bg_yl");
        } else {
          $("#sw1sw").removeClass("bg_yl").addClass("bg_bk");
        }
      break;
      case "label":
        $("#sw1txt").html(val);
        $("#sw1txt").show();
      break;
      }
    });
  });
}

function do_sw2( cmd ) {
  $.getJSON( "/cmd?sw2="+cmd, function( data ) {
    $.each( data, function( key, val ) {
      switch(key) {
      case "state":
        $("#sw2sw").html(val);
        if (val == "Ein") {
          $("#sw2sw").removeClass("bg_bk").addClass("bg_yl");
        } else {
          $("#sw2sw").removeClass("bg_yl").addClass("bg_bk");
        }
      break;
      case "label":
        $("#sw2txt").html(val);
        $("#sw2txt").show();
      break;
      }
    });
  });
}

function do_sw3( cmd ) {
  $.getJSON( "/cmd?sw3="+cmd, function( data ) {
    $.each( data, function( key, val ) {
      switch(key) {
      case "state":
        $("#sw3sw").html(val);
        if (val == "Ein") {
          $("#sw3sw").removeClass("bg_bk").addClass("bg_yl");
        } else {
          $("#sw3sw").removeClass("bg_yl").addClass("bg_bk");
        }
      break;
      case "label":
        $("#sw3txt").html(val);
        $("#sw3txt").show();
      break;
      }
    });
  });
}

function do_sw4( cmd ) {
  $.getJSON( "/cmd?sw4="+cmd, function( data ) {
    $.each( data, function( key, val ) {
      switch(key) {
      case "state":
        $("#sw4sw").html(val);
        if (val == "Ein") {
          $("#sw4sw").removeClass("bg_bk").addClass("bg_yl");
        } else {
          $("#sw4sw").removeClass("bg_yl").addClass("bg_bk");
        }
      break;
      case "label":
        $("#sw4txt").html(val);
        $("#sw4txt").show();
      break;
      }
    });
  });
}

function do_sw5( cmd ) {
  if ( ledmatrix_active == 1 ) {  
    $.get( "/cmd?intensity="+cmd, function( data ) {
      //alert("do_sw5: "+data);
    });
  }
}

function set_rgb() {
  $("#slider1label").html("Rot: "+red);
  $("#slider2label").html("Grün: "+green);
  $("#slider3label").html("Blau: "+blue);
  var r = red;
  var g = green << 8;
  var b = blue << 16;
  var rgb = r + g + b;
  $.get( "/cmd?setrgb="+rgb, function( data ) {
  });
}

function get_rgb() {
  $.get( "/cmd?getrgb", function( data ) {
    red   = data & 0x0000FF;
    green = data & 0x00FF00;
    green >>= 8;
    blue  = data & 0xFF0000;
    blue >>= 16;
    $("#slider1label").html("Rot: "+red);
    $("#slider1").val(red);
    $("#slider2label").html("Grün: "+green);
    $("#slider2").val(green);
    $("#slider3label").html("Blau: "+blue);
    $("#slider3").val(blue);
  });    
}

function set_ledpwm() {
  $("#slider1label").html("LED: "+ledpwm);
  $.get( "/cmd?setledpwm="+ledpwm, function( data ) {
  });
}

function get_ledpwm() {
  $.get( "/cmd?getledpwm", function( data ) {
    ledpwm   = data;
    $("#slider1label").html("LED: "+ledpwm);
    $("#slider1").val(ledpwm);
  });    
}

function do_restart() {
  var r = confirm("Wirklich neustarten?");
  if (r == true) {
    $.get("/restart", function(data) {
       alert(data);
    });
  }
}

function dellog() {
  $.get("/cmd?dellogfile", function(data) {
    alert(data);
  });
}

function store() {
  //alert($("input[name='saveeprom']:checked").val());  
  var mycmd = "/cmd?";
  mycmd = mycmd + "saveeprom=" + $("input[name='saveeprom']:checked").val();
  if ( $("#logfile").is(":checked") ) {
    mycmd = mycmd + "&logfile=1";
  } else {
    mycmd = mycmd + "&logfile=0";
  }
  if ( $("#console").is(":checked") ) {
    mycmd = mycmd + "&logger=1";
  } else {
    mycmd = mycmd + "&logger=0";
  }
  if ( $("#log_startup").is(":checked") ) {
    mycmd = mycmd + "&log_startup=1";
  } else {
    mycmd = mycmd + "&log_startup=0";
  }
  if ( $("#log_rf24").is(":checked") ) {
    mycmd = mycmd + "&log_rf24=1";
  } else {
    mycmd = mycmd + "&log_rf24=0";
  }
  if ( $("#log_sensor").is(":checked") ) {
    mycmd = mycmd + "&log_sensor=1";
  } else {
    mycmd = mycmd + "&log_sensor=0";
  }
  if ( $("#log_mqtt").is(":checked") ) {
    mycmd = mycmd + "&log_mqtt=1";
  } else {
    mycmd = mycmd + "&log_mqtt=0";
  }
  if ( $("#log_webcmd").is(":checked") ) {
    mycmd = mycmd + "&log_webcmd=1";
  } else {
    mycmd = mycmd + "&log_webcmd=0";
  }
  if ( $("#log_sysinfo").is(":checked") ) {
    mycmd = mycmd + "&log_sysinfo=1";
  } else {
    mycmd = mycmd + "&log_sysinfo=0";
  }
  //alert(mycmd);
  $.get(mycmd, function(data) {
    alert(data);
  });
}

function runCmd() {
  var cmd = $("#cmd").val()
  $.get('/cmd?'+cmd, function(data) {
//    alert(data);
  });
}

function intervalfunction() {
  if ( page == "index" ) {
    if ( ledmatrix_active == 1 ) {
      fill_ledmatrix();
    } 
    if ( neopixel_active == 1 ) {
      do_sw1("state");
      get_rgb();
    }
    if ( ledpwm_active == 1 ) {
      do_sw1("state");
      get_ledpwm();
    }
    if ( ledmatrix_active == 0 && neopixel_active == 0 && ledpwm_active == 0) {
      if ( sw1_active == 1) do_sw1('state');
    }  
    if ( sw2_active == 1) do_sw2('state');
    if ( sw3_active == 1) do_sw3('state');
    if ( sw4_active == 1) do_sw4('state');
    $.getJSON('/cmd?message1', function(data) {
      $.each( data, function( key, val ) {
        prozessJS( key, val );
      });
    });
    $.getJSON('/cmd?status', function(data) {
      $.each( data, function( key, val ) {
        prozessJS( key, val );
      });
    });
  }
  if ( page == "console" ) {
    $.get('console', function(data) {
      $('#console').html(data);
    });
  }
}
