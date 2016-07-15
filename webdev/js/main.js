var ws ;
var el_timer;
var Timer_sys;
var Timer_tinfo;
var counters={};
var isousc, iinst;
var elapsed = 0;
var socksrv;
var term;
var logs = true;

function formatSize(bytes) {
  if (bytes < 1024)  return bytes+' Bytes';
  if (bytes < (1024*1024)) return (bytes/1024).toFixed(0)+' KB';
  if (bytes < (1024*1024*1024)) return (bytes/1024/1024).toFixed(0)+' MB';
  return (bytes/1024/1024/1024).toFixed(0)+' GB';
}
function isJson(item) {
  if ((item.charAt(0)=='{' && item.slice(-1)=='}') || (item.charAt(0)=='[' && item.slice(-1)==']')) {
    return true;
  }
  return false
} 

function rowStyle(row, index) {
  //var classes=['active','success','info','warning','danger'];
  var flags=parseInt(row.fl,10);
  //if (flags & 0x80) return {classes:classes[4]};
  //if (flags & 0x02) return {classes:classes[3]};
  //if (flags & 0x08) return {classes:classes[1]};
  if (flags & 0x80) return {classes:'danger'};
  if (flags & 0x02) return {classes:'warning'};
  if (flags & 0x08) return {classes:'success'};
  return {};
}

function labelFormatter(value, row) {
  var flags=parseInt(row.fl,10);

  if ( typeof counters[value]==='undefined')
    counters[value]=1;
  if ( flags & 0x88 )
    counters[value]++;
  return value + ' <span class=\"badge\">'+counters[value]+'</span>';
  }

function valueFormatter(value, row) {
  if (row.na=="ISOUSC")
    isousc=parseInt(value);
  else if (row.na=="IINST") {
    var pb, pe, cl;
    iinst=parseInt(value);
    pe=parseInt(iinst*100/isousc);
    if (isNaN(pe))
      pe=0;
    cl='success';
    if (pe>70) cl ='info';
    if (pe>80) cl ='warning';
    if (pe>90) cl ='danger';

    cl = 'progress-bar-' + cl;
    if (pe>0)
      $('#scharge').text(pe+'%');
    if (typeof isousc!='undefined')
      $('#pcharge').text(iinst+'A / '+isousc+'A');
    $('#pbar').attr('class','progress-bar '+cl);
    $('#pbar').css('width', pe+'%');
  }
  return value;
}

function fileFormatter(value, row) {
  var fname = row.na;
  var htm = '';

  //ht+= '<button type="button" class="btn btn-xs btn-danger" title="Supprimer">';
  //ht+= '<span class="glyphicon glyphicon-trash" aria-hidden="true"></span>&nbsp;';
  //ht+= '</button>&nbsp;';
  //ht+= '<button type="button" class="btn btn-xs btn-primary" title="Télécharger">';
  //ht+= '<span class="glyphicon glyphicon-trash" aria-hidden="true"></span>&nbsp;';
  //ht+= '</button>&nbsp;';
  htm+= '<a href="' + fname + '">' + fname + '</a>';

  return htm;
}

function RSSIFormatter(value, row) {
  var rssi=parseInt(row.rssi);
  var signal = Math.min(Math.max(2*(rssi+100),0),100);
  var cl, htm;
  cl='success';
  if (signal<70) cl ='info';
  if (signal<50) cl ='warning';
  if (signal<30) cl ='danger';

  cl = 'progress-bar-' + cl;
  htm = "<div class='progress progress-tbl'>";
  htm+= "<div class='progress-bar "+cl+"' role='progressbar' aria-valuemin='0' aria-valuemax='100' ";
  htm+= "aria-valuenow='"+signal+"' style='width:"+signal+"%'>"+rssi+"dB</div></div>";
  return htm;
}

function Notify(mydelay, myicon, mytype, mytitle, mymsg) {
  $.notify(
    { icon:'glyphicon glyphicon-'+myicon,
      title:'&nbsp;<strong>'+mytitle+'</strong>',
      message:'<p>'+mymsg+'</p>',
    },{
      type:mytype,
      //showProgressbar: true,
      animate:{enter:'animated fadeInDown',exit:'animated fadeOutUp',delay:mydelay*1000}
    }
  );
}

function progressUpload(data) {
  if (data.lengthComputable) {
    var pe = (data.loaded/data.total*100).toFixed(0) ;
    $('#pfw').css('width', pe +'%');
    $('#psfw').text(formatSize(data.loaded)+' / '+formatSize(data.total));
  }
}

function waitReboot() {
  var url = location.protocol+'//'+location.hostname+(location.port ? ':'+location.port: '') + '/#tab_sys' ;
  $('#txt_srv').text('Tentative de connexion à '+url);
  $('#mdl_wait').modal();

  var thistimer = setInterval(function() {
    $.ajax({
      cache: false,
      type: 'GET',
      url: '/hb',
      timeout: 900,
      success: function(data, textStatus, XMLHttpRequest) {
      console.log(data);
      if (data === 'OK') {
        $('#mdl_wait').modal('hide');
        clearInterval(thistimer);
        window.location = url;
        location.reload();
        elapsed=0;
      }
      }
    });
    elapsed++;
    $('#txt_elapsed').text('Temps écoulé ' + elapsed + ' s');
  }
  ,1000);
}

function addZoneTemplate(id, zones) {
  // console.log('addZoneTemplate ', id, zones);
  var template = '<div class="col-sm-6 col-md-4 zone">'
         + '  <div style="min-height:80px;" class="thumbnail" data-zone="#zone#" title="Gestion de la zone #zone#">'
         + '    <div class="caption"><h5>Zone #zone#</h5><span class="icon iconCmd" style="font-size: 3.5em;"></span>'
         + '    <div>'
         + '      <a href="#" class="actions btn btn-default conf" role="button">Confort</a>'
         + '      <a href="#" class="actions btn btn-default eco" role="button">ECO</a>'
         + '      <a href="#" class="actions btn btn-default hg" role="button">hors gel</a>'
         + '      <a href="#" class="actions btn btn-default off" role="button">arrêt</a>'
         + '    </div>'
         + '  </div>'
         + '</div>';
  template = template.replace(/#zone#/g, id.replace('fp', ''));
  // console.log('template: ' + template);
  // console.log('tab_fp .panel-body', $('#tab_fp .panel-body'));
  $('#tab_fp .zones').append(template); // On ajoute la template dans le body du panel
  var $div = $('#tab_fp .zones div.zone:last');
  // console.log('div.zone last', $div);
  activeZone(id.replace('fp', ''), zones[id]); // On active le bon bouton et l'image associée
  // On ajoute le bind sur click sur les boutons
  $('.actions', $div).on('click', function(e) {
    e.stopPropagation();
    e.preventDefault();
    var $this = $(this),
    zone = $this.parents('.thumbnail').data('zone');
    if ($this.hasClass('conf')) {
      sendOrdre(zone, 'C');
    } else if ($this.hasClass('eco')) {
      sendOrdre(zone, 'E');
    } else if ($this.hasClass('hg')) {
      sendOrdre(zone, 'H');
    } else if ($this.hasClass('off')) {
      sendOrdre(zone, 'A');
    }
  });
}
function sendOrdre(id, ordre) {
  $('body').removeClass('loaded');
  // console.log('sendOrdre ', id, ordre);
  var request = id + ordre;
  if (id == false) {
    request = Array(8).join(ordre);
  }
  wsSend('$fp' + request);
}
function activeZone(id, state) {
  // console.log('activeZone', id, state);
  var $div = $('div.thumbnail[data-zone="'+id+'"]'),
  $icon = $('span.icon', $div),
  active,
  img;
  // console.log('div.zone: ', $div);
  // console.log('icon: ', $icon);
  switch (state) {
    case 'C':
      active = 'a.conf';
      img = 'jeedom-pilote-conf';
    break;
    case 'E':
      active = 'a.eco';
      img = 'jeedom-pilote-eco';
    break;
    case 'H':
      active = 'a.hg';
      img = 'jeedom-pilote-hg';
    break;
    case 'A':
      active = 'a.off';
      img = 'jeedom-pilote-off';
    break;
  }
  // console.log('active: %s - img: %s', active, img);
  $icon.empty();
  $('a.actions', $div).removeClass('active btn-success');
  $(active, $div).addClass('active btn-success');
  $icon.append('<i class="icon ' + img + '"></i>');
}

function timestampToHuman(seconds) {
  var time = {seconds: 0, minutes: 0, hours: 0, days: 0},
      index = 0,
      divider,
      dividers = [
        {title: 'seconds', divider: 1},
        {title: 'minutes', divider: 60},
        {title: 'hours', divider: 3600},
        {title: 'days', divider: 86400}
      ],
      str_time = '';
  while (seconds > dividers[index].divider && index < dividers.length-1) {
    index++;
  }
  for (; index >= 0; index--) {
    divider = dividers[index];
    time[divider.title] = Math.floor(seconds / divider.divider);
    seconds = seconds % divider.divider;
  }
  if (time.days > 0) {
    str_time += time.days + ' day';
    if (time.days > 1) {
      str_time += 's';
    }
    str_time += ' ';
  }
  for (index = 2; index >= 0; index--) {
    divider = dividers[index];
    if (time[divider.title] < 10) {
      time[divider.title] = '0' + time[divider.title].toString();
    }
  }
  str_time += time.hours + ':' + time.minutes + ':' + time.seconds;
  return str_time;
}

function refreshSystem(system_data) {
  //console.log('refreshSystem=' + system_data);
  if (system_data.length > 0 && system_data[0].na.toLowerCase() == 'uptime') {
    system_data[0].va = timestampToHuman(system_data[0].va);
  }
  $('#tab_sys_data').bootstrapTable('load', system_data);  
}

$('a[data-toggle=\"tab\"]').on('shown.bs.tab', function (e) {
  clearTimeout(Timer_sys);
  clearTimeout(Timer_tinfo);
  var target = $(e.target).attr("href")
  //console.log('activated ' + target );

// IE10, Firefox, Chrome, etc.
  if (history.pushState)
  window.history.pushState(null, null, target);
  else
  window.location.hash = target;

  if (target=='#tab_tinfo')  {
    $('#tab_tinfo_data').bootstrapTable('refresh',{silent:true, url:'/tinfo.json'});
  } else if (target=='#tab_sys') {
    wsSend('$system');
  } else if (target=='#tab_fs') {
    $.getJSON( "/spiffs.json", function(spiffs_data) {
      var pb, pe, cl;
      total= spiffs_data.spiffs[0].Total;
      used = spiffs_data.spiffs[0].Used;
      freeram = spiffs_data.spiffs[0].Ram;

      $('#tab_fs_data').bootstrapTable('load', spiffs_data.files, {silent:true, showLoading:true});

      pe=parseInt(used*100/total);
      if (isNaN(pe))
        pe=0;
      cl='success';
      if (pe>70) cl ='warning';
      if (pe>85) cl ='danger';

      cl = 'progress-bar-' + cl;
      if (pe>0)
        $('#sspiffs').text(pe+'%');
      $('#fs_use').text(formatSize(used)+' / '+formatSize(total));
      $('#pfsbar').attr('class','progress-bar '+cl);
      $('#pfsbar').css('width', pe+'%');

    })
    .fail(function() { console.log( "error while requestiong spiffs data" );  })
  } else if (target=='#tab_cfg') {
    $.getJSON( "/config.json", function(form_data) {
      $("#frm_config").autofill(form_data);
      })
      .fail(function() { console.log( "error while requestiong configuration data" ); })

    $('#tab_scan_data').bootstrapTable('refresh',{silent:true, showLoading:true, url:'/wifiscan.json'});
  }
  // Onglet de gestion des zones
  else if (target == '#tab_fp') {
    $('body').removeClass('loaded'); // On affiche le loader
    // On récupère l'état de toutes les zones
    wsSend('$fp');
  }
});

$('#tab_tinfo_data').on('load-success.bs.table', function (e, data) {
  //console.log('#tab_tinfo_data loaded');
  if ($('.nav-tabs .active > a').attr('href')=='#tab_tinfo')
    Timer_tinfo = setTimeout(function(){$('#tab_tinfo_data').bootstrapTable('refresh',{silent: true})},1000);
});
//$('#tab_sys_data').on('load-success.bs.table', function (e, data) { console.log('#tab_sys_data loaded');  })
// On actualise les données système 1 seconde après avoir affiché celles reçues
$('#tab_sys_data').on('post-body.bs.table', function (e, data) {
  //console.log('#tab_sys_data loaded');
  if ($('.nav-tabs .active > a').attr('href') == '#tab_sys') {
    Timer_sys = setTimeout(function() {wsSend('$system');}, 1000);
  }
});
$('#tab_fs_data').on('load-success.bs.table', function (e, data) {
  //console.log('#tab_fs_data loaded');
})
.on('load-error.bs.table', function (e, status) {
  //console.log('Event: load-error.bs.table');
  // myTimer=setInterval(function(){myRefresh()},5000);
});

$('#tab_scan').on('click-row.bs.table', function (e, name, args) {
  var $form = $('#tab_cfg');
  $('#ssid').val(name.ssid);
  setTimeout(function(){$('#psk').focus()},500);
  $('#tab_scan').modal('hide');
});
$('#btn_scan').click(function () {
  $('#tab_scan_data').bootstrapTable('refresh',{url:'/wifiscan.json',showLoading:false,silent:true});
});
$('#btn_reset').click(function () {
  $.post('/factory_reset');
  waitReboot();
  return false;
});
$('#btn_reboot').click(function () {
  $.post('/reset');
  waitReboot();
  return false;
});

$(document)
  .on('change', '.btn-file :file', function() {
    var input = $(this),
    numFiles = input.get(0).files ? input.get(0).files.length : 1,
    label = input.val().replace(/\\/g, '/').replace(/.*\//, '');
    input.trigger('fileselect', [numFiles, label]);
  })
  .on('show.bs.collapse', '.panel-collapse', function () {
    var $span = $(this).parents('.panel').find('span.pull-right.glyphicon');
    $span.removeClass('glyphicon-chevron-down').addClass('glyphicon-chevron-up');
  })
  .on('hide.bs.collapse', '.panel-collapse', function () {
    var $span = $(this).parents('.panel').find('span.pull-right.glyphicon');
    $span.removeClass('glyphicon-chevron-up').addClass('glyphicon-chevron-down');
  });

$('#frm_config').validator().on('submit', function (e) {
  // everything looks good!
  if (!e.isDefaultPrevented()) {
    e.preventDefault();
    //console.log("Form Submit");

    $.post('/config_form.json', $("#frm_config").serialize())
      .done( function(msg, textStatus, xhr) {
      Notify(2, 'ok', 'success', 'Enregistrement effectué', xhr.status+' '+msg);
      })
      .fail( function(xhr, textStatus, errorThrown) {
      Notify(4, 'remove', 'danger', 'Erreur lors de l\'enregistrement', xhr.status+' '+errorThrown);
      }
  );
  }
});

$('#file_fw').change(function() {
  var $txt = $('#txt_upload_fw');
  var $btn = $('#btn_upload_fw');
  var ok = true;
  var f = this.files[0];
  var name = f.name.toLowerCase();
  var size = f.size;
  var type = f.type;
  var html = 'Fichier:' + name + '&nbsp;&nbsp;type:' + type + '&nbsp;&nbsp;taille:' + size + ' octets'
  console.log('name='+name);
  console.log('size'+size);
  console.log('type='+type);

  $('#pgfw').removeClass('show').addClass('hide');
  $('#sfw').text( name + ' : ');

  if (!f.type.match('application/octet-stream')) {
    Notify(3, 'remove', 'danger', 'Type de fichier non conforme', 'Le fichier de mise à jour doit être un fichier binaire');
    ok = false;
  //} else if (name!="remora_soft.cpp.bin" && name!="remora_soft.spiffs.bin") {
  } else if (! /^remora_soft.*.bin$/i.test(name) ) {
    Notify(5, 'remove', 'danger', 'Nom de fichier incorrect', 'Le fichier de mise à jour doit être nommé <ul><li>remora_soft.*.bin (Micro-logiciel) ou</li><li>remora_soft.spiffs.bin (Système de fichiers)</li></ul>');
    ok = false;
  }
  if (ok) {
    $btn.removeClass('hide');
    if ( name==="remora_soft.spiffs.bin") {
      label = 'Mise à jour SPIFFS';
    } else {
      label = 'Mise à jour Micro-Logiciel';
    }
    $btn.val(label);
    $('#fw_info').html('<strong>' + label + '</strong> ' + html);
  } else {
    $txt.attr('readonly', '');
    $txt.val('');
    $txt.attr('readonly', 'readonly');
    $btn.addClass('hide');
  }
  return ok;
});

$('#btn_upload_fw').click(function() {
  var formData = new FormData($('#frm_fw')[0]);
  $.ajax({
    url: '/update',
    type: 'POST',
    data: formData,
    cache: false,
    contentType: false,
    processData: false,
    xhr: function() {
      var myXhr = $.ajaxSettings.xhr();
      if (myXhr.upload)
        myXhr.upload.addEventListener('progress',progressUpload, false);
      return myXhr;
    },
    beforeSend: function () {
      $('#pgfw').removeClass('hide');
    },
    success: function(msg, textStatus, xhr) {
      Notify(2, 'floppy-saved', 'success','Envoi de la mise à jour terminé', '<strong>'+xhr.status+'</strong> '+msg);
      waitReboot();
    },
    error: function(xhr, textStatus, errorThrown) {
      $('#pfw').removeClass('progress-bar-success').addClass('progress-bar-danger');
      Notify(4, 'floppy-remove', 'danger', 'Erreur lors de la mise à jour du fichier ' + name,
        '<strong>' + xhr.status + '</strong> ' + errorThrown);
    }
  });
});

function wsSend(message) {
  if (ws.readyState===1)
    ws.send( message );
}

function refreshPage() {
  var url = document.location.toString();  
  var tab = url.split('#')[1];
  if (typeof(tab)==='undefined') 
    tab='tab_tinfo';
  //console.log('Tab activation='+tab);
  $('.nav-tabs a[href=#'+tab+']').tab('show');  
  $('a[data-toggle="tab"][href="' + tab + '"]').trigger("shown.bs.tab");
  console.log('refreshPage('+tab+')')
  //$('.nav-tabs a[href=#'+tab+']').on('shown', function(e){window.location.hash=e.target.hash;});   
}
function ts() {
  var myDate = new Date();
  var ts='';
  if (myDate.getHours()<10) ts+='0';
  ts+=myDate.getHours() + ':';
  if (myDate.getMinutes()<10) ts+='0';
  ts+=myDate.getMinutes() + ':';
  if (myDate.getSeconds()<10) ts+='0';
  ts+=myDate.getSeconds() + ' ';
  return ts;
}

window.onload = function() {
  var url = document.location.toString();
  var full = location.protocol+'//'+location.hostname+(location.port ? ':'+location.port: '');
  console.log (url);
  console.log (full);
  console.log (location.port);

  if (url.match('#')) {
    $('.nav-tabs a[href=#' + url.split('#')[1] + ']').tab('show');
  }

  $('.nav-tabs a[href=#' + url.split('#')[1] + ']').on('shown', function(e) {
    window.location.hash = e.target.hash;
  });

  // Instanciate the terminal object
  term = $('#term').terminal( function(command, term) {
      if (command == '!help') {
        this.echo("available commands are [[b;cyan;]!help], [[b;cyan;]!log], [[b;cyan;]!nolog]\n" + 
                  "each terminal commmand need to be prefixed by [[b;cyan;]!]\n" + 
                  "if not they will be send and interpreted by NRJMeter\n" +
                   "examples: [[b;cyan;]!nolog]  disable log output to this console\n" +
                  "          [[b;cyan;]!log]    enable log output to this console"); 

      } else if (command == '!nolog') {
        this.echo("[[b;cyan;]Log disabled]");
        logs = false;
      } else if (command == '!log') {
        this.echo("[[b;cyan;]Log enabled]");
        logs = true;
      } else if (command == '!log?') {
        this.echo("[[b;cyan;]Log="+logs+"]");
      } else {
        // Send raw command to client
        wsSend(command);
      }
    }, 
    // Default terminal settings and greetings
    { prompt: '[[b;red;]>] ', 
      checkArity : false,
      greetings: "Type [[b;cyan;]help] to see NRJMeter available commands\n" +
                 "Type [[b;cyan;]!help] to see terminal commands"
    }
  );

  // open a web socket
  socksrv = 'ws://'+location.hostname+':'+location.port+'/ws';
  //socksrv = 'ws://192.168.1.93:80/ws';
  console.log('socket server='+socksrv);
  //ws = new WebSocket("ws://localhost:8081");
  //ws = new WebSocket(socksrv);
  ws = new ReconnectingWebSocket(socksrv);
  //ws.debug = true;

  // When Web Socket is connected
  ws.onopen = function(evt) {
    console.log( 'WS Connect');
    //Notify(1, 'eye-open', 'success', 'Connected to nrjmeter', '');
    //$('.nav-tabs a[href=#'+document.location.toString().split('#')[1]+']').tab('show');  
    $('#mdl_wait').modal('hide'); 
    $('#connect').text('connected').removeClass('label-danger').addClass('label-success');  
    var srv = location.hostname;
    term.echo("WS[Connected] to [[;green;]ws:/"+srv+':'+location.port+"/ws]");
    term.set_prompt("[[b;green;]"+srv+" #]");
    elapsed=0;
    clearInterval(el_timer);
    refreshPage();
  };

  // When websocket is closed.
  ws.onclose = function(code, reason) { 
    $('#connect').text('disconnected').removeClass('label-success').addClass('label-danger'); 
    term.error('WS[Disconnected] '+code+' '+reason);
    term.set_prompt("[[b;red;]>]");
  };

  // When websocket message
  ws.onmessage = function (evt) { 
    console.log( 'WS Received data'/*+evt.data*/);
    if (isJson(evt.data)) {
      // console.log( 'WS Received JSON');
      var obj, msg, data;
      try {
        obj =  JSON.parse(evt.data);
        msg = obj.message;
        data = obj.data;
      } catch(e) {
        Notify(4, 'floppy-remove', 'danger', 'Erreur lors du parsing des données reçues via websocket',
        '<strong>' + e.toString() + '</strong>');
        console.error(e, evt.data);
      }
    
      if (logs == true) {
        var str = ts()+"WS["+msg+"] "+ JSON.stringify(data);
        str = str.replace(/\]/g, "&#93;");
        term.echo("[[;darkgrey;]"+str+"]");
      }
      if (msg == 'system') { 
        refreshSystem(data);
      } else if (msg == 'log' && logs == true)  {
        data.replace(/\]/g, "&#93;");
        term.echo("[[;darkgrey;]" + ts() + data + "]");
      } else if (msg == 'fp') {
        // console.log('data length: ', Object.keys(data).length);
        if (Object.keys(data).length >= 7) {
          $('#tab_fp .zones').empty(); // On vide l'espace d'affichage des zones
          for (var k in data) {
            addZoneTemplate(k, data); // On ajoute l'affichage d'une zone
          }
          $('body').addClass('loaded'); // On masque le loader
          // On ajoute un bind sur les boutons d'action généraux
          $('#tab_fp .all .actions').unbind('click').click(function(e) {
            e.stopPropagation();
            e.preventDefault();
            var $this = $(this);
            if ($this.hasClass('conf')) {
              sendOrdre(false, 'C');
            } else if ($this.hasClass('eco')) {
              sendOrdre(false, 'E');
            } else if ($this.hasClass('hg')) {
              sendOrdre(false, 'H');
            } else if ($this.hasClass('off')) {
              sendOrdre(false, 'A');
            }
          });
        } else {
          for (var k in data) {
            var id = k.replace('fp', '');
            activeZone(id, data[k]);
          }
        }
      }
    } else {
      // Raw Data (mainly response)
      console.log( 'WS Received RAW');
      var str = evt.data;
      str = str.replace(/\]/g, "&#93;");
      term.echo("[[;darkgrey;]"+str+"]");
    }
  };

  // When Web Socket error
  ws.onerror = function(evt)  {
    term.error( ts()+'WS[error]');
  };

  // $("#cfg_led_bright")
  //   .on('slideStop',function(){ wsSend('$rgbb:'+$('#cfg_led_bright').slider('getValue'));});

  // $("#cfg_debug").bootstrapSwitch({ onColor:"success", offColor:"danger"});
  // $("#cfg_rgb").bootstrapSwitch({ onColor:"success", offColor:"danger"});
  // $("#cfg_oled").bootstrapSwitch({ onColor:"success", offColor:"danger"});
  // $("#cfg_ap").bootstrapSwitch({ onColor:"success", offColor:"danger"});
  // $("#cfg_wifi").bootstrapSwitch({ onColor:"success", offColor:"danger"});
  // $("#cfg_static").bootstrapSwitch({ onColor:"success", offColor:"danger"});
}

$('#btn_test').click(function(){ waitReboot(); });