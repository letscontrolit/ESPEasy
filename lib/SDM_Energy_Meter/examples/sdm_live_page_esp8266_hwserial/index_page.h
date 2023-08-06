const char index_page[] PROGMEM = R"=====(
<!DOCTYPE HTML>
<HTML>
  <HEAD>
    <META name='viewport' content='width=device-width, initial-scale=1'>
    <TITLE>SDM live POWER table</TITLE>
    <SCRIPT>
    var xmlHttp=createXmlHttpObject();
    function createXmlHttpObject(){
     if(window.XMLHttpRequest){
        xmlHttp=new XMLHttpRequest();
     }else{
        xmlHttp=new ActiveXObject('Microsoft.XMLHTTP');
     }
     return xmlHttp;
    }
    function process(){
      if(xmlHttp.readyState==0 || xmlHttp.readyState==4){
        xmlHttp.open('PUT','xml',true);
        xmlHttp.onreadystatechange=handleServerResponse;
        xmlHttp.send(null);
      }
      setTimeout('process()',2000);
    }
    function handleServerResponse(){
     if(xmlHttp.readyState==4 && xmlHttp.status==200){
       xmlResponse=xmlHttp.responseXML;
       for(i=0;i<5;i++){
        xmldoc=xmlResponse.getElementsByTagName('response'+i)[0].firstChild.nodeValue;
        document.getElementById('resp'+i).innerHTML=xmldoc;
       }
       xmldoc=xmlResponse.getElementsByTagName('freeh')[0].firstChild.nodeValue;
       document.getElementById('freeheap').innerHTML=xmldoc;
       xmldoc=xmlResponse.getElementsByTagName('rst')[0].firstChild.nodeValue;
       document.getElementById('rstreason').innerHTML=xmldoc;
     }
    }
    </SCRIPT>
    <STYLE>
      h1 {
        font-size: 120%;
        color: blue;
        margin: 0 0 10px 0;
      }
       table{
        border-collapse: collapse;
      }     
      table, th, td {
        text-align: center;
        border: 1px solid blue;
      }
      tr:nth-child(even) {background-color: #f2f2f2}
    </STYLE>
  </HEAD>
  <BODY onload='process()'>
    <CENTER>
      <H1>SDM live POWER table</H1>
      <TABLE BORDER=1>
        <TR><TH title="VOLTAGE">VOLTAGE</TH><TD><A id='resp0'></A></TD><TD>V</TD></TR>
        <TR><TH title="CURRENT">CURRENT</TH><TD><A id='resp1'></A></TD><TD>A</TD></TR>
        <TR><TH title="POWER">POWER</TH><TD><A id='resp2'></A></TD><TD>W</TD></TR>
        <TR><TH title="POWER FACTOR">POWER FACTOR</TH><TD><A id='resp3'></A></TD><TD>PF</TD></TR>
        <TR><TH title="FREQUENCY">FREQUENCY</TH><TD><A id='resp4'></A></TD><TD>Hz</TD></TR>
        <TR><TH title="FREE HEAP">FREE HEAP</TH><TD><A id='freeheap'></A></TD><TD>bytes</TD></TR>
        <TR><TH title="LAST RESET REASON">LAST RESET REASON</TH><TD colspan="2"><A id='rstreason'></A></TD></TR>
      </TABLE>
    </CENTER>
  </BODY>
</HTML>
)=====";
