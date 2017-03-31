#include <SPI.h>
#include <SdFat.h>
SdFat SD;

/* SKETCH PRO ZAPIS POTREBNYCH SOUBORU NA PAMETOVOU KARTU */

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// CELY SKETCH NEJDE NAHRAT DO ARDUINA NAJEDNOU (malo pameti)
// JE TREBA ZAPINAT (1) POLOZKY A NAHRAVAT PO JEDNE
  #define DUMP 0
  
  /* 1. up*/
  #define LABELS 0
  #define AJAX 0

  /* 3. up*/
  #define FIRST 1 // file2 - file7

  /* 4. up*/
  #define SECOND 0 // file8 - file18

  /* 5. up*/
  #define THIRD 0 // file 19 - xxx

  /* 6. up*/
  #define FORTH 0 // file 

  /* 7. up*/
  #define FIFTH 0

  
  #define POPISKY 0
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//  NAZVY SOUBORU JSOU NEPREHLEDNE KVULI TOMU, ABY BYLY CO NEJKRATSI A BRALI TAK CO NEJMENE PAMETI ARDUINA!

void log_vypadek() {
  const char *name = "vypadky.log";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);
  
  File log_vypadek = SD.open(name, FILE_WRITE);
  log_vypadek.close(); delay(10);
  
  log_vypadek = SD.open(name, FILE_WRITE);
  log_vypadek.println("cidlo;   zacatek_vypadku;   konec_vypadku;");  
  log_vypadek.close(); delay(10);
  printfile(name);
}

void printfile(const char *name) {
  SD.begin(4);
  File dataFile = SD.open(name);
  // if the file is available, write to it:
  if (dataFile) {
    while (dataFile.available()) {
      Serial.write(dataFile.read());
    }
    dataFile.close(); delay(10);
  }
  Serial.println("");
  Serial.println("");
}

void confs() {
  const char * name[] = {"confauth", "confmail"};
  SD.begin(4);

  for (int i = 0; i < 2; ++i) {
    SD.remove(name[i]);
    File file = SD.open(name[i], FILE_WRITE);
    file.println("EMPTY_CONF");
    file.close(); delay(10);
    
    Serial.print("----");
    Serial.print(name[i]);
    Serial.println("----");
    printfile(name[i]);
  }
  
}
#if POPISKY
void labels() {
  for (int i=0; i<14; ++i) {
    char name[50];
    char numb[5];
    
    itoa(i, numb, 10);
    
    strcpy(name, "lab");
    strcat(name, numb);
    strcat(name, "");
    
    SD.begin(4);
    SD.remove(name);
    // Create file
    File file = SD.open(name, FILE_WRITE);
    file.close(); delay(10);
    file = SD.open(name, FILE_WRITE);
    file.print("TANGER");
    file.println(numb);
    file.close(); delay(10);
    printfile(name);
  }
}

#endif

void dump(const char * file) {
  Serial.print("Dumping file '");
  Serial.print(file);
  Serial.println("':");
  printfile(file);
}

/* FILE START */

#if FIRST 
void file0() {
  const char *name = "main0";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);
  File myFile1 = SD.open(name, FILE_WRITE);
  myFile1.println("HTTP/1.1 200 OK");
  myFile1.println("Content-Type: text/html");
  myFile1.println("");
  myFile1.println("<!DOCTYPE HTML>");
  myFile1.println("<html>");
  myFile1.println("<head>");
  myFile1.println("<meta charset='utf-8'>");
  myFile1.println("<title>Tanger temperature</title>");

  myFile1.println("<style type='text/css'>");
  myFile1.println("" \
  "#min, #max {width: 70px};\n" \
  "#day, #month, #year, #hour, #minute, #second {width: 30px};\n" \
  "#year {width: 40px};" \
  "");
  myFile1.println("</style>");
  
  myFile1.println("<link rel='stylesheet' href='http://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/css/bootstrap.min.css'>");
  myFile1.println("<script src='https://ajax.googleapis.com/ajax/libs/jquery/1.12.0/jquery.min.js'></script>");
  myFile1.println("<script src='http://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/js/bootstrap.min.js'></script>");
  myFile1.close(); delay(10);

  printfile(name);
  
}


void file1() {
  const char *name = "main1";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);
  File myFile1 = SD.open(name, FILE_WRITE);
  myFile1.println("</head>");
  myFile1.println("<body style='background:yellow' onload='getAjaxValues()'>");
  myFile1.println("<div class='container-fluid'>");
  myFile1.println("<div class='col-sm-2'>");
  myFile1.println("<h2>Menu</h2>");
  myFile1.println("<a href='/_home'>Aktuální teploty</a></br/></br/>");
  myFile1.println("<a href='/_csv'>CSV exporty čidel</a></br/></br/>");
  myFile1.println("<a href='/_xml' target='_blank'>XML export</a></br/></br/>");
  myFile1.println("<a href='/_log' target='_blank'>Poslední výpadky</a> (<a href='/_log91587clear' onclick=\"return confirm('Opravdu smazat log?')\">clear</a>)</br/></br/>");
  myFile1.println("<a href='/_nastSens'>Nastavení senzorů</a></br/></br/>");
  myFile1.println("<a href='/_tmpLim'>Globální kritické teploty</a></br/></br/>");
  myFile1.println("<a href='/_nastCas'>Nastaveni času</a></br/></br/>");
  myFile1.println("<a href='/_nast-mail'>Nastaveni emailu</a></br/></br/>");
  myFile1.println("<a href='/_nast-mer'>Nastaveni mereni</a></br/></br/>");
  myFile1.println("<a href='/_nast-eth'>Nastaveni ethernetu</a></br/></br/>");
  myFile1.println("</div>");
  myFile1.println("<div class='col-sm-10'>");
  myFile1.close(); delay(10);

  printfile(name);
}


void file2() {
  const char *name = "main2";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);
  File myFile2 = SD.open(name, FILE_WRITE);
  myFile2.println("</div>");
  myFile2.println("</div>");
  myFile2.println("</body>");
  myFile2.println("</html>");
  myFile2.close(); delay(10);

  printfile(name);
}






void file3() {
  const char *name = "vypadek";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);
  File myFile3 = SD.open(name, FILE_WRITE);
  myFile3.println("<table>");
  myFile3.println("<tr><td><b>@01:</b></td><td>@02 - @03 ( @04 )</td></tr>");
  myFile3.println("</table>");
  myFile3.close(); delay(10);

  printfile(name);
}


void file4() {
  const char *name = "stat1";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);

    // Create file
  File myFile4 = SD.open(name, FILE_WRITE);
  myFile4.close(); delay(10);
  
  myFile4 = SD.open(name, FILE_WRITE);
  myFile4.println("<table class='table'>");
  myFile4.println("<tr><td id='aj_infoL'></td><td id='aj_info'></td></tr>");
  myFile4.println("<tr><td>Aktualni cas:</td><td id='aj_nowtime'></td></tr>");
  myFile4.println("<tr><td>Uptime:</td><td id='aj_uptime'></td></tr>");
  myFile4.println("<tr><td>SRAM:</td><td id='aj_sram'></td></tr>");
  myFile4.println("<tr><td>Pocet aktivnich cidel:</td><td id='aj_cidpoc'></td></tr>");
  myFile4.close(); delay(10);

  printfile(name);
}




void file4_1() {
  const char *name = "stat2";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);

  // Create file
  File myFile4 = SD.open(name, FILE_WRITE);
  myFile4.close(); delay(10);
  
  myFile4 = SD.open(name, FILE_WRITE);
  
  myFile4.println("<tr><td>RTC teplota:</td><td id='aj_rtctemp'></td></tr>");
  myFile4.println("<tr><td><i>Globalni max teplota:</i></td><td id='aj_glob_max'></td></tr>");
  myFile4.println("<tr><td><i>Globalni min teplota:</i></td><td id='aj_glob_min'></td></tr>");
  myFile4.println("<tr><td>Prumerna teplota:</td><td id='aj_avg_temp'></td></tr>");
  myFile4.println("<tr><td>Nejvyssi teplota:</td><td id='aj_max_temp'></td></tr>");
  myFile4.println("<tr><td>Nejmensi teplota:</td><td id='aj_min_temp'></td></tr>");
  myFile4.println("</table>");
  myFile4.close(); delay(10);

  printfile(name);
}


void file5() {
  const char *name = "setmail";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);

  // Create file
  File myFile5 = SD.open(name, FILE_WRITE);
  myFile5.close(); delay(10);

  myFile5 = SD.open(name, FILE_WRITE);
  
  myFile5.println("<div class='col-sm-12'>" \
    "<center><h1>Nastavení e-mailu</h1></center></div>");
  myFile5.println("<div class='col-sm-12'>");
  
  myFile5.println("<div class='form-inline'><form action='_q7' method='get'><div class='form-group'>" \
    "<label for='smtp'>SMTP server:</label>" \
    "<input type='text' style='width:50px' class='form-control' name='ip1' id='ip1' value='@01'><span>.</span>" \
    "<input type='text' style='width:50px' class='form-control' name='ip2' id='ip2' value='@02'><span>.</span>" \
    "<input type='text' style='width:50px' class='form-control' name='ip3' id='ip3' value='@03'><span>.</span>" \
    "<input type='text' style='width:50px' class='form-control' name='ip4' id='ip4' value='@04'><span>:</span>" \
    "<input type='text' style='width:60px' class='form-control' name='port' id='port' value='@05'>" \
    "</div>" \
    "<button type='submit' class='btn btn-default'>Nastavit</button>" \
    "</form>" \
    "</div><br/>");



    
  myFile5.close(); delay(10);

  printfile(name);

}



void file5_1() {
  const char *name = "setmail1";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);

  // Create file
  File myFile5 = SD.open(name, FILE_WRITE);
  myFile5.close(); delay(10);

  myFile5 = SD.open(name, FILE_WRITE);
  
  
  myFile5.println("<div class='form-inline'>" \
    "<form action='_q8' method='get'>" \
    "<div class='form-group'>" \
    "<label for='smtp'>Autorizace:</label>" \
    "<input type='checkbox' class='form-control' name='xauth' id='xauth' @01>" \
    "</div>" \
    "<div class='form-group'>" \
    "<label for='user'>User:</label>" \
    "<input type='text' class='form-control' name='user' id='user' value='@02'>" \
    "</div>" \
    "<div class='form-group'>" \
    "<label for='pass'>Password:</label>" \
    "<input type='password' class='form-control' name='pass' id='pass' value='@03'>" \
    "</div>" \
    "<button type='submit' class='btn btn-default'>Nastavit</button>" \
    "</form>" \
    "</div><br/>");
    
  myFile5.close(); delay(10);

  printfile(name);

}




void file5_2() {
  const char *name = "setmail2";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);

  // Create file
  File myFile5 = SD.open(name, FILE_WRITE);
  myFile5.close(); delay(10);

  myFile5 = SD.open(name, FILE_WRITE);
  
    
  myFile5.println("<div class='form-inline'>" \
    "<form action='_q9' method='get'>" \
    "<div class='form-group'>" \
    "<label for='from'>Odesílatel:</label>" \
    "<input type='email' class='form-control' name='from' value='@01' id='from'>" \
    "</div>" \
    "<div class='form-group'>" \
    "<label for='to'>Příjemce:</label>" \
    "<input type='email' class='form-control' name='to' value='@02' id='to'>" \
    "</div>" \
    "<button type='submit' class='btn btn-default'>Nastavit</button>" \
    "</form>" \
    "<a href='/_q10'>Send alert test mail</a>&nbsp;" \ 
    "<a href='/_q11'>Send normal test mail</a>" \ 
    "</div></div>");
    
  myFile5.close(); delay(10);

  printfile(name);

}




void file6() {
  const char *name = "temp_nadp";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);

  // Create file
  File myFile6 = SD.open(name, FILE_WRITE);
  myFile6.close(); delay(10);

  myFile6 = SD.open(name, FILE_WRITE);
  myFile6.println("<center><h1>Teploty</h1></center>");
  myFile6.close(); delay(10);

  printfile(name);

}





#endif

#if SECOND

void file7() {
  const char *name = "setlimit";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);

  // Create file
  File myFile7 = SD.open(name, FILE_WRITE);
  myFile7.close(); delay(10);

  myFile7 = SD.open(name, FILE_WRITE);
  myFile7.println("<div class='col-sm-12'>");
  myFile7.println("<center><h1>Nastavení limitních teplot</h1></center>");
  myFile7.println("</div>");
  myFile7.println("<div class='col-sm-12'>");
  myFile7.println("<form action='_q15' method='get'>");
  myFile7.println("<div class='alert alert-info'>Rozmezí @01 až @02<br/></div>");
  myFile7.println("<fieldset class='form-group'>");
  myFile7.println("<label for='min_krit'>Min. kritická:</label>");
  myFile7.println("<input type='text' class='form-control' name='min' id='min' value='@03'>");
  myFile7.println("</fieldset>");

  myFile7.println("<fieldset class='form-group'>");
  myFile7.println("<label for='max_krit'>Max. kritická:</label>");
  myFile7.println("<input type='text' class='form-control' name='max' id='max' value='@04'>");
  myFile7.println("</fieldset>");

  myFile7.println("<fieldset class='form-group'>");
  myFile7.println("<label for='rozd'>Rozdíl pro vrácení do normál stavu:</label>");
  myFile7.println("<input type='text' class='form-control' name='rozd' id='rozd' value='@05'>");
  myFile7.println("</fieldset>");

  myFile7.println("<button type='submit'>Nastavit</button>");
  myFile7.println("</form></div>");


  myFile7.close(); delay(10);

  printfile(name);
}

void file8() {
  const char *name = "settime";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);

  // Create file
  File myFile8 = SD.open(name, FILE_WRITE);
  myFile8.close(); delay(10);

  myFile8 = SD.open(name, FILE_WRITE);
  myFile8.println("<div class='col-sm-12'>");
  myFile8.println("<center><h1>Nastavení času</h1></center>");
  myFile8.println("</div>");
  myFile8.println("<div class='col-sm-12'>");

  // NTP nastaveni casu
  myFile8.println("<b>Aktuální čas:</b><br/><span id='aj_nowtime'></span><br/><form action='_q3' method='get'>" \
                  "<button type='submit'>NTP Sync</button><span> (Poslední autosync: <span id='aj_lastsync'></span>, interval: <span id='aj_interval'></span>, další za: <span id='aj_nextsync'></span>)" \
                  "</span></form>");
  myFile8.println("<script>");
  myFile8.println("function getAjaxValues() {");
    myFile8.println("nocache = \"?nocache=\" + Math.random() * 1000000;");
    myFile8.println("var rq = new XMLHttpRequest();");
    myFile8.println("rq.onreadystatechange = function() {");
      myFile8.println("if (this.readyState == 4 && this.status == 200) {");
        myFile8.println("document.getElementById('aj_nowtime').innerHTML = this.responseXML.getElementsByTagName('nowtime')[0].childNodes[0].nodeValue;");
        myFile8.println("document.getElementById('aj_lastsync').innerHTML = this.responseXML.getElementsByTagName('lastsync')[0].childNodes[0].nodeValue;");
        myFile8.println("document.getElementById('aj_interval').innerHTML = this.responseXML.getElementsByTagName('interval')[0].childNodes[0].nodeValue;");
        myFile8.println("document.getElementById('aj_nextsync').innerHTML = this.responseXML.getElementsByTagName('nextsync')[0].childNodes[0].nodeValue;");
    myFile8.println("}}");
    myFile8.println("rq.open(\"GET\", \"_ajaxtime\" + nocache, true);");
    myFile8.println("rq.send(null);");
    myFile8.println("setTimeout('getAjaxValues()', 900);");
  myFile8.println("}"); 
  myFile8.println("</script>");
  myFile8.println("</div>");


  myFile8.close(); delay(10);

  printfile(name);
}



void file8_1() {
  const char *name = "setAdvInt";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);

  // Create file
  File myFile8 = SD.open(name, FILE_WRITE);
  myFile8.close(); delay(10);

  myFile8 = SD.open(name, FILE_WRITE);
  // Rucni nastaveni casu
  myFile8.println("<div class='col-sm-12'>");
  myFile8.println("<form class='form-inline' action='_@01' method='get'>");
  myFile8.println("<b>@02</b></br>");
  myFile8.println("<fieldset class='form-group'>");
  myFile8.println("<label for='day'>Dnů:</label>");
  myFile8.println("<input type='text' class='form-control' name='day' id='day' value='@03'>");
  myFile8.println("<label for='hour'>Hodin:</label>");
  myFile8.println("<input type='text' class='form-control' name='hour' id='hour' value='@04'>");
  myFile8.println("<label for='minute'>Minut:</label>");
  myFile8.println("<input type='text' class='form-control' name='minute' id='minute' value='@05'>");
  myFile8.println("<label for='second'>Sekund:</label>");
  myFile8.println("<input type='text' class='form-control' name='second' id='second' value='@06'>");
  myFile8.println("<button type='submit'>Nastavit</button>");
  myFile8.println("</fieldset>");
  myFile8.println("</form>");
  myFile8.println("</div>");

  
  myFile8.close(); delay(10);

  printfile(name);
}

void file8_2() {
  const char *name = "settime2";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);

  // Create file
  File myFile8 = SD.open(name, FILE_WRITE);
  myFile8.close(); delay(10);

  myFile8 = SD.open(name, FILE_WRITE);
  // Rucni nastaveni casu
  myFile8.println("<div class='col-sm-12'>");
  myFile8.println("<form class='form-inline' action='_q5' method='get'>");
  myFile8.println("<b>Ruční nastavení času:</b></br>");
  myFile8.println("<fieldset class='form-group'>");
  myFile8.println("<label for='day'>Den:</label>");
  myFile8.println("<input type='text' class='form-control' name='day' id='day' value='@01'>");
  myFile8.println("<label for='month'>Mesic:</label>");
  myFile8.println("<input type='text' class='form-control' name='month' id='month' value='@02'>");
  myFile8.println("<label for='year'>Rok:</label>");
  myFile8.println("<input type='text' class='form-control' name='year' id='year' value='@03'>");
  myFile8.println("<label for='hour'>Hodina:</label>");
  myFile8.println("<input type='text' class='form-control' name='hour' id='hour' value='@04'>");
  myFile8.println("<label for='minute'>Minuta:</label>");
  myFile8.println("<input type='text' class='form-control' name='minute' id='minute' value='@05'>");
  myFile8.println("<label for='second'>Sekunda:</label>");
  myFile8.println("<input type='text' class='form-control' name='second' id='second' value='@06'>");
  myFile8.println("<button type='submit'>Nastavit</button>");
  myFile8.println("</fieldset>");
  myFile8.println("</form>");
  myFile8.println("</div>");

  
  myFile8.close(); delay(10);

  printfile(name);
}

void file8_3() {
  const char *name = "setntp";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);

  // Create file
  File myFile8 = SD.open(name, FILE_WRITE);
  myFile8.close(); delay(10);

  myFile8 = SD.open(name, FILE_WRITE);
  // Rucni nastaveni casu
  myFile8.println("<div class='col-sm-12'>");
  myFile8.println("<form class='form-inline' action='_q6' method='get'>");
  myFile8.println("<b>NTP server:</b></br>");
  myFile8.println("<fieldset class='form-group'>");
  myFile8.println("<label for='server'>Server:</label>");
  myFile8.println("<input type='text' class='form-control' name='server' id='server' value='@01'>");
  myFile8.println("<label for='port'>Port:</label>");
  myFile8.println("<input type='text' class='form-control' name='port' id='port' value='@02'>");
  myFile8.println("<button type='submit'>Nastavit</button>");
  myFile8.println("</fieldset>");
  myFile8.println("</form>");
  myFile8.println("</div>");

  
  myFile8.close(); delay(10);

  printfile(name);
  
}




void file9() {
  const char *name = "rdok";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);

  // Create file
  File myFile9 = SD.open(name, FILE_WRITE);
  myFile9.close(); delay(10);

  myFile9 = SD.open(name, FILE_WRITE);
  myFile9.println("<h2 style='color:green'>Uložení @01 proběhlo úspěšně.</h2>");


  myFile9.close(); delay(10);

  printfile(name);

}

void file10() {
  const char *name = "redirfai";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);

  // Create file
  File myFile10 = SD.open(name, FILE_WRITE);
  myFile10.close(); delay(10);

  myFile10 = SD.open(name, FILE_WRITE);
  myFile10.println("<h2 style='color:red'>Uložení @01 se nezdařilo.</h2>");


  myFile10.close(); delay(10);

  printfile(name);
}

void file11() {
  const char *name = "xml";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);

  // Create file
  File myFile11 = SD.open(name, FILE_WRITE);
  myFile11.close(); delay(10);

  myFile11 = SD.open(name, FILE_WRITE);
  myFile11.println("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>");
  myFile11.println("<temperatures>");
  myFile11.println("<time>");
  myFile11.print("<year>");
  myFile11.print("@01");
  myFile11.println("</year>");
  myFile11.print("<month>");
  myFile11.print("@02");
  myFile11.println("</month>");
  myFile11.print("<day>");
  myFile11.print("@03");
  myFile11.println("</day>");
  myFile11.print("<hour>");
  myFile11.print("@04");
  myFile11.println("</hour>");
  myFile11.print("<minute>");
  myFile11.print("@05");
  myFile11.println("</minute>");
  myFile11.print("<second>");
  myFile11.print("@06");
  myFile11.println("</second>");
  myFile11.println("</time>");
  myFile11.println("<global_limits>");
  myFile11.print("<kriticky_limit_min>");
  myFile11.print("@07");
  myFile11.println("</kriticky_limit_min>");
  myFile11.print("<kriticky_limit_max>");
  myFile11.print("@08");
  myFile11.println("</kriticky_limit_max>");
  myFile11.println("</global_limits>");
  myFile11.println("<summary>");
  myFile11.print("<aktivnich_cidel>");
  myFile11.print("@09");
  myFile11.println("</aktivnich_cidel>");
  myFile11.print("<kritickych_teplot>");
  myFile11.print("@10");
  myFile11.println("</kritickych_teplot>");
  myFile11.print("<aktualni_prumerna>");
  myFile11.print("@11");
  myFile11.println("</aktualni_prumerna>");
  myFile11.print("<aktualni_nejvetsi>");
  myFile11.print("@12");
  myFile11.println("</aktualni_nejvetsi>");
  myFile11.print("<aktualni_nejmensi>");
  myFile11.print("@13");
  myFile11.println("</aktualni_nejmensi>");
  myFile11.println("</summary>");


  myFile11.close(); delay(10);

  printfile(name);


}

void file12() {
  const char *name = "lsnadp";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);

  // Create file
  File myFile12 = SD.open(name, FILE_WRITE);
  myFile12.close(); delay(10);

  myFile12 = SD.open(name, FILE_WRITE);
  myFile12.println("<table class='table'><tr><th></th><th>PIN:</th><th>Teplota (&deg;C):</th><th>Nastavení limitu:</th><th>Poslední výpadek</th><th>Popis</th></tr>");


  myFile12.close(); delay(10);

  printfile(name);

}

void file13() {
  const char *name = "lbs";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);

  // Create file
  File myFile13 = SD.open(name, FILE_WRITE);
  myFile13.close(); delay(10);

  myFile13 = SD.open(name, FILE_WRITE);
  myFile13.println("<tr><td>Teplota @01:</td>");
  myFile13.println("<td>A@02</td>");
  myFile13.print("<td>");
  
  myFile13.print("<span id='s@03'>@04</span><br/><div id='progdiv@05' class='progress'>" \
  "<div id='progbar@06' role='progressbar'>" \
  "</div>" \
  "</div>");

  myFile13.println("</td><td>");
  myFile13.println("<form class='form-inline' action='_q0' method='get'><fieldset class='form-group'>");
  myFile13.println("<input type='hidden' name='id' value='@07'>");


  myFile13.close(); delay(10);

  printfile(name);

}

void file14() {
  const char *name = "lbs2";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);
  
  File myFile14 = SD.open(name, FILE_WRITE);
  myFile14.close(); delay(10);
  
  myFile14 = SD.open(name, FILE_WRITE);
  myFile14.println("<input name='xu' type='checkbox'\" onchange=\"{this.form.submit();}\" checked>");
  myFile14.println("<label for='min'>Min:</label>");
  myFile14.println("<input id='min' type='text' name='min' value='@01'><b> (návrat při t > <span id='ret'>@02</span>),</b>");
  myFile14.println("<label for='max'>Max:</label>");
  myFile14.println("<input id='max' type='text' name='max' value='@03'><b> (návrat při t < <span id='ret'>@04</span>)</b><button type='ok'>Nastavit</button></form>");
  myFile14.close(); delay(10);

  printfile(name);
  
  
}

void file15() {
  const char *name = "lbs2d";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);
  
  File myFile15 = SD.open(name, FILE_WRITE);
  myFile15.close(); delay(10);
  
  myFile15 = SD.open(name, FILE_WRITE);
  myFile15.println("<input name='xu' type='checkbox' onchange=\"{this.form.submit();}\">");
  myFile15.println("<label for='min'>Min:</label><span><b>@01</b></span>");
  myFile15.println("<input type='hidden' id='min' name='min' value='@02'><b>(návrat při t > @03),</b>");
  myFile15.println("<label for='max'>Max:</label><span><b>@04</b></span>");
  myFile15.println("<input type='hidden' id='max' name='max' value='@05'><b>(návrat při t < @06)</b></form>");
  
  myFile15.close(); delay(10);

  printfile(name);
  
  
}




#endif

#if THIRD

void file16() {
  const char *name = "mailconf";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);
  
  File myFile16 = SD.open(name, FILE_WRITE);
  myFile16.close(); delay(10);
  
  myFile16 = SD.open(name, FILE_WRITE);
  myFile16.println("<label for='mail'>E-mail pro zasilani upozorneni:</label>");
  myFile16.println("<input id='mai;' type='text' name='mail' value='@01'>");
  myFile16.println("<button type='ok'>Nastavit</button></form>");
  
  myFile16.close(); delay(10);
  printfile(name);
}

void file17() {
  const char *name = "setsmtp";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);

  // Create file
  File myFile17 = SD.open(name, FILE_WRITE);
  myFile17.close(); delay(10);

  myFile17 = SD.open(name, FILE_WRITE);
  myFile17.println("<div class='col-sm-12'>");
  myFile17.println("<center><h1>Nastavení zasílání e-mailů</h1></center>");
  myFile17.println("</div>");

  myFile17.println("<div class='col-sm-12'>");
  myFile17.println("<form action='_md-up' method='get'>");
  myFile17.println("<fieldset class='form-group'>");
  myFile17.println("<label for='si'>Po kolika sekundách překročení posílat varovné e-maily (0 - 65535):");
  myFile17.println("<input type='text' class='form-control' name='md' id='md' value='@01'>");
  myFile17.println("</fieldset>");
  myFile17.println("<button type='submit'>Nastavit</button>");
  myFile17.println("</form></div></div>");

  myFile17.close(); delay(10);

  printfile(name);

}

void file18() {
  const char *name = "setsmtp2";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);

  // Create file
  File myFile18 = SD.open(name, FILE_WRITE);
  myFile18.close(); delay(10);

  myFile18 = SD.open(name, FILE_WRITE);
  myFile18.println("<div class='col-sm-12'>");
  myFile18.println("<center><h1>Nastavení zasílání e-mailů</h1></center>");
  myFile18.println("</div>");

  myFile18.println("<div class='col-sm-12'>");
  myFile18.println("<form action='_md-up' method='get'>");
  myFile18.println("<fieldset class='form-group'>");
  myFile18.println("<label for='si'>Po kolika sekundách překročení posílat varovné e-maily (0 - 65535):");
  myFile18.println("<input type='text' class='form-control' name='md' id='md' value='@01'>");
  myFile18.println("</fieldset>");
  myFile18.println("<button type='submit'>Nastavit</button>");
  myFile18.println("</form></div></div>");

  myFile18.close(); delay(10);

  printfile(name);

}

void file19() {
  const char *name = "setsens0";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);
  
  File file = SD.open(name, FILE_WRITE);
  file.close(); delay(10);

  file = SD.open(name, FILE_WRITE);
  file.println("<div class='col-sm-12'>");
  file.println("<center><h1>Nastavení senzorů</h1></center>");
  file.println("</div>");

  file.println("<div class='col-sm-12'>");
  file.println("<table class='table'><thead><tr><th>Sensor:</th><th>PIN:</th><th>Popis:</th></tr></thead><tbody>");
  file.close(); delay(10);

  printfile(name);
}

    
void file20() {
  const char *name = "setsens";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);

  // Create file
  File file = SD.open(name, FILE_WRITE);
  file.close(); delay(10);

  file = SD.open(name, FILE_WRITE);

  file.println("</select></fieldset><button type='submit'>Změnit</button></form></td><td><form action='_q13' method='get'>");
  file.println("<fieldset class='form-group'>");
  file.println("<input type='hidden' class='form-control' name='id' id='id' value='@01'>");
  file.println("<input type='text' class='form-control' name='popis' id='popis' value='@02'>");
  file.println("</fieldset>");
  file.println("<button type='submit'>Změnit</button>");
  file.println("</form></td>");

  file.close(); delay(10);

  printfile(name);

}







void file21() {
  const char *name = "setsens1";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);
  
  File file = SD.open(name, FILE_WRITE);
  file.close(); delay(10);

  file = SD.open(name, FILE_WRITE);
  file.println("</tbody>");
  file.println("</table>");
  file.println("</div>");
  file.close(); delay(10);

  printfile(name);
}

#endif

#if FORTH

void file22() {
  const char *name = "mereni";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);
  
  File file = SD.open(name, FILE_WRITE);
  file.close(); delay(10);

  file = SD.open(name, FILE_WRITE);
  
  file.println("<div class='col-sm-12'>");
  file.println("<center><h1>Nastavení měření</h1></center>");
  file.println("</div>");
  
  file.println("<div class='col-sm-12'>");
  file.println("<form action='_q16' method='get'>");
  file.println("<fieldset class='form-group'>");
  file.println("<label for='si'>Interval mezi měřeními v ms (milisekundách): </label>");
  file.println("<input type='text' class='form-control' name='si' id='si' value='@01'>");
  file.println("<label for='sk'>Konstanta napeti: </label>");
  file.println("<input type='text' class='form-control' name='sk' id='sk' value='@02'>");
  file.println("<button type='submit'>Nastavit</button>");
  file.println("</fieldset>");
  file.println("</form>");
  file.println("</div>");
  file.close(); delay(10);

  printfile(name);
}


void file23() {
  const char *name = "ethernet";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);
  
  File file = SD.open(name, FILE_WRITE);
  file.close(); delay(10);

  file = SD.open(name, FILE_WRITE);
  file.println("<div class='col-sm-12'><center><h1>Nastavení ethernetu</h1></center></div>");
  file.println("<div class='col-sm-12'>");
    file.println("<table class='table table-condensed'>" \
    "<tr><td>IP:</td><td>@01</td></tr>" \
    "<tr><td>MAC:</td><td>@02</td></tr>" \
    "</table>");

  file.println("<form action='_q17' method='get'><div class='form-inline'>" \
    "<div class='form-group'>" \
    "<label for='smtp'>Statická IP:</label>" \
    "<input type='checkbox' class='form-control' name='xstat' id='xstat' @03>" \
    "</div>" \
    "</div><br/>");
  
  file.close(); delay(10);

  printfile(name);
}
void file24() {
  const char *name = "eth1";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);
  
  File file = SD.open(name, FILE_WRITE);
  file.close(); delay(10);

  file = SD.open(name, FILE_WRITE);
  file.println("<div class='form-inline'><div class='form-group'>" \
    "<label for='smtp'>IP:</label>" \
    "<input type='text' style='width:50px' class='form-control' name='ip1' id='ip1' value='@01'><span>.</span>" \
    "<input type='text' style='width:50px' class='form-control' name='ip2' id='ip2' value='@02'><span>.</span>" \
    "<input type='text' style='width:50px' class='form-control' name='ip3' id='ip3' value='@03'><span>.</span>" \
    "<input type='text' style='width:50px' class='form-control' name='ip4' id='ip4' value='@04'>" \
    "</div></div>");
  file.close(); delay(10);

  printfile(name);
}
void file25() {
  const char *name = "eth2";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);
  
  File file = SD.open(name, FILE_WRITE);
  file.close(); delay(10);

  file = SD.open(name, FILE_WRITE);
  file.println("<div class='form-inline'><div class='form-group'>" \
    "<label for='smtp'>Maska:</label>" \
    "<input type='text' style='width:50px' class='form-control' name='mask1' id='mask1' value='@01'><span>.</span>" \
    "<input type='text' style='width:50px' class='form-control' name='mask2' id='mask2' value='@02'><span>.</span>" \
    "<input type='text' style='width:50px' class='form-control' name='mask3' id='mask3' value='@03'><span>.</span>" \
    "<input type='text' style='width:50px' class='form-control' name='mask4' id='mask4' value='@04'>" \
    "</div></div>");
  file.close(); delay(10);

  printfile(name);
}
void file26() {
  const char *name = "eth3";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);
  
  File file = SD.open(name, FILE_WRITE);
  file.close(); delay(10);

  file = SD.open(name, FILE_WRITE);
  file.println("<div class='form-inline'><div class='form-group'>" \
    "<label for='smtp'>Brána:</label>" \
    "<input type='text' style='width:50px' class='form-control' name='gw1' id='gw1' value='@01'><span>.</span>" \
    "<input type='text' style='width:50px' class='form-control' name='gw2' id='gw2' value='@02'><span>.</span>" \
    "<input type='text' style='width:50px' class='form-control' name='gw3' id='gw3' value='@03'><span>.</span>" \
    "<input type='text' style='width:50px' class='form-control' name='gw4' id='gw4' value='@04'>" \
    "</div></div>");
  file.close(); delay(10);

  printfile(name);
}
void file27() {
  const char *name = "eth4";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);
  
  File file = SD.open(name, FILE_WRITE);
  file.close(); delay(10);

  file = SD.open(name, FILE_WRITE);
  file.println("<div class='form-inline'><div class='form-group'>" \
    "<label for='dns'>DNS:</label>" \
    "<input type='text' style='width:50px' class='form-control' name='dns1' id='dns1' value='@01'><span>.</span>" \
    "<input type='text' style='width:50px' class='form-control' name='dns2' id='dns2' value='@02'><span>.</span>" \
    "<input type='text' style='width:50px' class='form-control' name='dns3' id='dns3' value='@03'><span>.</span>" \
    "<input type='text' style='width:50px' class='form-control' name='dns4' id='dns4' value='@04'>" \
    "</div></div>" \
    "<button type='submit' class='btn btn-default'>Nastavit</button>(Dojde k restartu.)" \
    "</form>");
  file.close(); delay(10);

  printfile(name);
}
void file28() {
  const char *name = "eth0";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);
  
  File file = SD.open(name, FILE_WRITE);
  file.close(); delay(10);

  file = SD.open(name, FILE_WRITE);
  file.println("</form>");
  file.println("</div>");
  file.close(); delay(10);

  printfile(name);
}

#endif


#if FIFTH


void graph() {
  const char *name = "graph";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);
  
  File file = SD.open(name, FILE_WRITE);
  file.close(); delay(10);

  file = SD.open(name, FILE_WRITE);
  file.println("</div class='col-sm-12'>");
  file.println("<canvas  id='graph' width='600' height='300'></canvas>");
  file.println("</div>");
  file.close(); delay(10);
  
  printfile(name);
}

void partSens() {
  const char *name = "pSensID";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);
  
  File file = SD.open(name, FILE_WRITE);
  file.close(); delay(10);

  file = SD.open(name, FILE_WRITE);
  file.println("<tr><td>Sensor@01");
  file.println("</td><td>");
  
  file.println("<form action='_q14' method='get'>");
  file.println("<fieldset class='form-group'>");
  file.println("<input type='hidden' class='form-control' name='id' id='id' value='@02'>");
  file.println("<select class='form-control' name='pinout' id='pinout'>");
  file.close(); delay(10);

  printfile(name);
}


void partOpt() {
  const char *name = "pSensOpt";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);
  
  File file = SD.open(name, FILE_WRITE);
  file.close(); delay(10);

  file = SD.open(name, FILE_WRITE);
  file.println("<option @01>@02</option>");
  file.close(); delay(10);

  printfile(name);
}


void csv0() {
  const char *name = "csv0";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);
  
  File file = SD.open(name, FILE_WRITE);
  file.close(); delay(10);

  file = SD.open(name, FILE_WRITE);
  file.println("<div class='col-sm-12'><center><h1>CSV export</h1></center>");
  file.close(); delay(10);

  printfile(name);
  
}

void csv1() {
  const char *name = "csv1";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);
  
  File file = SD.open(name, FILE_WRITE);
  file.close(); delay(10);

  file = SD.open(name, FILE_WRITE);
  file.println("<a href='_sens?id=@01' targer='_blank'>Sensor @02</a><br/>");
  file.close(); delay(10);

  printfile(name);
  
}

void divend() {
  const char *name = "divEND";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);
  
  File file = SD.open(name, FILE_WRITE);
  file.close(); delay(10);

  file = SD.open(name, FILE_WRITE);
  file.println("</div>");
  file.close(); delay(10);

  printfile(name);
  
}


void xmlsens() {  
  const char *name = "xmls";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);
  
  File file = SD.open(name, FILE_WRITE);
  file.close(); delay(10);

  file = SD.open(name, FILE_WRITE);
  file.println("<temp>@01</temp>");
  file.println("<limit typ=\"@02\"><min>@03</min><max>@04</max></limit>");
  file.println("<alert>@05</alert>");
  file.close(); delay(10);

  printfile(name);
  
}


void air_cond0() {
  const char *name = "aircond0";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);
  
  File file = SD.open(name, FILE_WRITE);
  file.close(); delay(10);

  file = SD.open(name, FILE_WRITE);
  file.println("<table class='table table-condensed'><tr><td>Klimatizace 0: <span id='acstat0'></span></td></tr><tr><td><form role='form' action='_q1' method='get'><div class='form-group'><select class='form-control' name='set' id='set'><option value='0'>OFF</option><option value='17'>17</option><option value='18'>18</option><option value='19'>19</option><option value='20'>20</option><option value='21'>21</option><option value='22'>22</option><option value='23'>23</option><option value='24'>24</option><option value='25'>25</option><option value='26'>26</option><option value='27'>27</option><option value='28'>28</option><option value='29'>29</option><option value='30'>30</option></select><button type='submit'>Nastavit</button></div></form></td></tr>");

  file.close(); delay(10);

  printfile(name);
}

void air_cond1() {
  const char *name = "aircond1";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);
  
  File file = SD.open(name, FILE_WRITE);
  file.close(); delay(10);

  file = SD.open(name, FILE_WRITE);
  file.println("<tr><td>Klimatizace 1: <span id='acstat1'></span></td></tr><tr><td><form role='form' action='_q2' method='get'><div class='form-group'><select class='form-control' name='set' id='set'><option value='0'>OFF</option><option value='17'>17</option><option value='18'>18</option><option value='19'>19</option><option value='20'>20</option><option value='21'>21</option><option value='22'>22</option><option value='23'>23</option><option value='24'>24</option><option value='25'>25</option><option value='26'>26</option><option value='27'>27</option><option value='28'>28</option><option value='29'>29</option><option value='30'>30</option></select><button type='submit'>Nastavit</button></div></form></td></tr></table>");
  
  file.close(); delay(10);

  printfile(name);
}



#endif

#if LABELS

void redir_script() {
  const char *name = "scrd";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);
  
  SD.remove("ntpcf");

  File a = SD.open("ntpcf", FILE_WRITE);
  a.println("time.nist.gov");
  a.close(); delay(10);

  // Create file
  File myFile8 = SD.open(name, FILE_WRITE);
  myFile8.close(); delay(10);

  myFile8 = SD.open(name, FILE_WRITE);
  myFile8.println("<meta http-equiv=\"refresh\" content=\"@01;url='/_q12'\">");

  
  myFile8.close(); delay(10);

  printfile(name);
  printfile("scrd");
}

void label_temp_ok() {
  const char *name = "lbTm0";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);

  // Create file
  File myFile8 = SD.open(name, FILE_WRITE);
  myFile8.close(); delay(10);

  myFile8 = SD.open(name, FILE_WRITE);
  myFile8.println("Teplota v normě na všech senzorech od: ");

  
  myFile8.close(); delay(10);

  printfile(name);
}

void label_temp_fail() {
  const char *name = "lbTmF";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);

  // Create file
  File myFile8 = SD.open(name, FILE_WRITE);
  myFile8.close(); delay(10);

  myFile8 = SD.open(name, FILE_WRITE);
  myFile8.println("Teplota překročena (alespoň na jednom senzoru) od: ");

  
  myFile8.close(); delay(10);

  printfile(name);
}

void label_mail_subject_fail() {
    const char *name = "labMaF";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);

  // Create file
  File myFile8 = SD.open(name, FILE_WRITE);
  myFile8.close(); delay(10);

  myFile8 = SD.open(name, FILE_WRITE);
  myFile8.println("TANGER Serverovna - Překročena teplota ");

  
  myFile8.close(); delay(10);

  printfile(name);
  
}

void label_mail_subject_ok() {
    const char *name = "labMaO";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);

  // Create file
  File myFile8 = SD.open(name, FILE_WRITE);
  myFile8.close(); delay(10);

  myFile8 = SD.open(name, FILE_WRITE);
  myFile8.println("TANGER Serverovna - Teplota ");

  
  myFile8.close(); delay(10);

  printfile(name);
  
}

void label_mail_subject_ok2() {
    const char *name = "labMaO2";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);

  // Create file
  File myFile8 = SD.open(name, FILE_WRITE);
  myFile8.close(); delay(10);

  myFile8 = SD.open(name, FILE_WRITE);
  myFile8.println(" opět v normálu");

  
  myFile8.close(); delay(10);

  printfile(name);
  
}

#endif

#if AJAX



void file_script0() {
  const char *name = "sc0";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");
  
  SD.begin(4);
  SD.remove(name);

  
  // Create file
  File myFile = SD.open(name, FILE_WRITE);
  myFile.close(); delay(10);
  
  myFile = SD.open(name, FILE_WRITE);
  
  myFile.println("<script>");

  myFile.println("function ge(s) {");
  myFile.println("return document.getElementById(s);");
  myFile.println("};");
  
  myFile.println("function getAjaxValues() {");

  myFile.println("nocache = \"?nocache=\" + Math.random() * 1000000;");
  myFile.println("var rq = new XMLHttpRequest();");
  myFile.println("rq.onreadystatechange = function() {");
    myFile.println("if (this.readyState == 4 && this.status == 200) {");
      myFile.println("if (this.responseXML != null) {");
  myFile.println("ge('aj_infoL').innerHTML = this.responseXML.getElementsByTagName('infolab')[0].childNodes[0].nodeValue;");
  myFile.println("ge('aj_info').innerHTML = this.responseXML.getElementsByTagName('info')[0].childNodes[0].nodeValue;");
  myFile.println("ge('aj_nowtime').innerHTML = this.responseXML.getElementsByTagName('nowtime')[0].childNodes[0].nodeValue;");
  myFile.println("ge('aj_uptime').innerHTML = this.responseXML.getElementsByTagName('uptime')[0].childNodes[0].nodeValue;");
  myFile.println("ge('aj_sram').innerHTML = this.responseXML.getElementsByTagName('ram')[0].childNodes[0].nodeValue;");
  myFile.println("ge('aj_glob_max').innerHTML = this.responseXML.getElementsByTagName('globmax')[0].childNodes[0].nodeValue +  '&degC';");
  myFile.println("ge('aj_glob_min').innerHTML = this.responseXML.getElementsByTagName('globmin')[0].childNodes[0].nodeValue +  '&degC';");


  myFile.close(); delay(10);

  printfile(name);

}

void file_script1() {
  const char *name = "sc1";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");
  
  SD.begin(4);
  SD.remove(name);

  
  // Create file
  File myFile = SD.open(name, FILE_WRITE);
  myFile.close(); delay(10);
  
  myFile = SD.open(name, FILE_WRITE);
  myFile.println("ge('aj_cidpoc').innerHTML = this.responseXML.getElementsByTagName('cidpoc')[0].childNodes[0].nodeValue;");
  myFile.println("ge('aj_rtctemp').innerHTML = this.responseXML.getElementsByTagName('rtctemp')[0].childNodes[0].nodeValue;");
  myFile.println("ge('aj_avg_temp').innerHTML = this.responseXML.getElementsByTagName('avgtemp')[0].childNodes[0].nodeValue +  '&degC';");
  myFile.println("ge('aj_max_temp').innerHTML = this.responseXML.getElementsByTagName('maxtemp')[0].childNodes[0].nodeValue +  '&degC';");
  myFile.println("ge('aj_min_temp').innerHTML = this.responseXML.getElementsByTagName('mintemp')[0].childNodes[0].nodeValue +  '&degC';");
  myFile.println("ge('acstat0').innerHTML = this.responseXML.getElementsByTagName('aircond0')[0].childNodes[0].nodeValue;");
  myFile.println("ge('acstat1').innerHTML = this.responseXML.getElementsByTagName('aircond1')[0].childNodes[0].nodeValue;");
  myFile.println("var element;");
  myFile.println("for (i = 0; i < 14; i++) {");
  myFile.println("element = ge('s' + i);");
  myFile.println("if (element != null) {");
  myFile.println("element.innerHTML = this.responseXML.getElementsByTagName('temp' + i)[0].childNodes[0].nodeValue;");
  myFile.println("var proc = this.responseXML.getElementsByTagName('alertproc' +i)[0].childNodes[0].nodeValue");

  myFile.close(); delay(10);

  printfile(name);

}


void file_script2() {
  const char *name = "sc2";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");
  
  SD.begin(4);
  SD.remove(name);

  
  // Create file
  File myFile = SD.open(name, FILE_WRITE);
  myFile.close(); delay(10);
  
  myFile = SD.open(name, FILE_WRITE);
  myFile.println("if (parseFloat(proc) == 0.0 || parseFloat(proc) == 100.0) {");
  myFile.println("ge('progbar' + i).className = \"progress-bar progress-bar-danger\"");
  myFile.println("element.style.color = 'red'} else {");
  myFile.println("ge('progbar' + i).className = \"progress-bar progress-bar-success\"");
  myFile.println("element.style.color = 'green'}");
  
  myFile.println("var progw = this.responseXML.getElementsByTagName('alertproc' + i)[0].childNodes[0].nodeValue");
  myFile.println("if (progw == '-1') {");
  myFile.println("$('#progdiv' + i).attr('style', 'display:none');");
  myFile.println("} else {");
  myFile.println("$('#progbar' + i).attr('style', 'width:' + this.responseXML.getElementsByTagName('alertproc' + i)[0].childNodes[0].nodeValue + '%') }");

  // posledni vypadek
  myFile.println("ge('vyp' + i).innerHTML = this.responseXML.getElementsByTagName('lastbreakout' + i)[0].childNodes[0].nodeValue;");
  // popis senzoru
  myFile.println("ge('pop' + i).innerHTML = this.responseXML.getElementsByTagName('label' + i)[0].childNodes[0].nodeValue;");

  
  myFile.println("}}");
      myFile.println("}");
    myFile.println("}");
  myFile.println("}");
  myFile.println("rq.open(\"GET\", \"_ajs\" + nocache, true);");
  myFile.println("rq.send(null);");
  myFile.println("setTimeout('getAjaxValues()', 1000);");
  myFile.println("}");
  
  myFile.println("</script>");

  myFile.close(); delay(10);

  printfile(name);

}


void ajax_time() {
  const char *name = "tim.aj";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);
  File myFile3 = SD.open(name, FILE_WRITE);
  
  myFile3.println("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>");
  myFile3.println("<ajax>");
  myFile3.println("<nowtime>@01</nowtime>");
  myFile3.println("<lastsync>@02</lastsync>");;
  myFile3.println("<interval>@03</interval>");;
  myFile3.println("<nextsync>@04</nextsync>");
  myFile3.println("</ajax>");
  
  myFile3.close(); delay(10);

  printfile(name); 
  
}

void ajax_summary() {
  const char *name = "sum.ajax";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);
  File myFile3 = SD.open(name, FILE_WRITE);
  
  myFile3.println("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>");
  myFile3.println("<ajax>");
  myFile3.println("<infolab>@01</infolab>");
  myFile3.println("<info>@02</info>");
  myFile3.println("<nowtime>@03</nowtime>");
  myFile3.println("<uptime>@04</uptime>");
  myFile3.println("<ram>@05</ram>");
  myFile3.println("<cidpoc>@06</cidpoc>");
  myFile3.println("<rtctemp>@07</rtctemp>");
  
  myFile3.close(); delay(10);

  printfile(name); 
}


void ajax_summary2() {
  const char *name = "sum2.ajax";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);
  File myFile3 = SD.open(name, FILE_WRITE);
  myFile3.println("<avgtemp>@01</avgtemp>");
  myFile3.println("<maxtemp>@02</maxtemp>");
  myFile3.println("<mintemp>@03</mintemp>");
  myFile3.println("<globmax>@04</globmax>");
  myFile3.println("<globmin>@05</globmin>");
  myFile3.println("<aircond0>@06</aircond0>");
  myFile3.println("<aircond1>@07</aircond1>");
  
  myFile3.close(); delay(10);

  printfile(name); 
}

void ajax_sensor() {
  const char *name = "sens.ajax";
  Serial.print("---");
  Serial.print(name);
  Serial.println("---");

  SD.begin(4);
  SD.remove(name);
  File myFile3 = SD.open(name, FILE_WRITE);
  
  myFile3.println("<temp@01>@02</temp@03>");
  myFile3.println("<lastbreakout@04>@05</lastbreakout@06>");
  myFile3.println("<label@07>@08</label@09>");
  myFile3.println("<alertproc@10>@11</alertproc@12>");
  
  myFile3.close(); delay(10);

  printfile(name); 
}

#endif


void setup() {
  // disable ethernet module

  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  delay(50);
  // initial serial link
  Serial.begin(9600);

  #if DUMP
dump("settime");
dump("settime2");
dump("rdok");
dump("rdfa");
dump("xml");
dump("lsnadp");
dump("lbs");
dump("lbs2");
dump("lbs2d");
dump("mailconf");
dump("setsmtp");
dump("SETSMTP2");
dump("MAIN1");
dump("MAIN2");
dump("VYPADEK");
dump("STAT1");
dump("STAT2");
dump("SETMAIL");
dump("SETMAIL1");
dump("SETMAIL2");
dump("REFRESH");
dump("SETLIMIT");
dump("SETSENS0");
dump("SETSENS");
dump("SETSENS2");
dump("SETSENS1");
dump("MERENI");
dump("ETHERNET");
dump("ETH1");
dump("ETH2");
dump("ETH3");
dump("ETH4");
dump("ETH0");

  #endif

  #if POPISKY
    labels();
  #endif

  #if FIRST
    file0();
    file1();
    file2();
    file3();
    file4();
    file4_1();
    file5();
    file5_1();
    file5_2();
    file6();
  #endif

  #if SECOND
    file7();
    file8();
    file8_1();
    file8_2();
    file8_3();
    file9();
    file10();
    file11();
    file12();
    file13();
    file14();
    file15();
  #endif
  
  #if THIRD
    file16();
    file17();
    file18();
    file19();
    file20();
    file21();
  #endif

  #if FORTH
    file22();
    file23();
    file24();
    file25();
    file26();
    file27();
    file28();
  #endif

  #if FIFTH
    partSens();
    partOpt();
    csv0();
    csv1();
    divend();
    xmlsens();
    graph();
    air_cond0();
    air_cond1();
  #endif

  #if LABELS
    label_temp_fail();
    label_temp_ok();
    redir_script();

    label_mail_subject_fail();
    label_mail_subject_ok();
    label_mail_subject_ok2();
  #endif


  #if AJAX
    file_script0();
    file_script1();
    file_script2();
    ajax_summary();
    ajax_summary2();
    ajax_sensor();
    ajax_time();
  #endif
    
}



void loop() {
}



