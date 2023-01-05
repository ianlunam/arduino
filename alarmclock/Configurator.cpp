#include "Alarm.h"
#include "Configurator.h"

// Configurator::Configurator() {}


String Configurator::pageTop = R"=====(
<html>
  <head>
    <!-- meta http-equiv='refresh' content='5'/ -->
    <title>Alarm Clock</title>
    <style>
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }
    </style>
    <script>
function checkWeekdays() {
  document.getElementById("monday").checked = "checked";
  document.getElementById("tuesday").checked = "checked";
  document.getElementById("wednesday").checked = "checked";
  document.getElementById("thursday").checked = "checked";
  document.getElementById("friday").checked = "checked";
  document.getElementById("saturday").checked = "";
  document.getElementById("sunday").checked = "";
}
function checkWeekends() {
  document.getElementById("monday").checked = "";
  document.getElementById("tuesday").checked = "";
  document.getElementById("wednesday").checked = "";
  document.getElementById("thursday").checked = "";
  document.getElementById("friday").checked = "";
  document.getElementById("saturday").checked = "checked";
  document.getElementById("sunday").checked = "checked";
}
function checkEveryday() {
  document.getElementById("monday").checked = "checked";
  document.getElementById("tuesday").checked = "checked";
  document.getElementById("wednesday").checked = "checked";
  document.getElementById("thursday").checked = "checked";
  document.getElementById("friday").checked = "checked";
  document.getElementById("saturday").checked = "checked";
  document.getElementById("sunday").checked = "checked";
}
function uncheckEveryday() {
  document.getElementById("monday").checked = "";
  document.getElementById("tuesday").checked = "";
  document.getElementById("wednesday").checked = "";
  document.getElementById("thursday").checked = "";
  document.getElementById("friday").checked = "";
  document.getElementById("saturday").checked = "";
  document.getElementById("sunday").checked = "";
}
    </script>
  </head>
  <body>
    <h1>Hello from the Alarm Clock!</h1>
    <form action="/save">
)=====";

String Configurator::pageBottom PROGMEM = R"=====(
        <input type="submit" value="Save"/>
      <br/>
      <p>Shortcuts</p>
      <input type="button" value="Weekdays" onclick="checkWeekdays()"/><br/>
      <input type="button" value="Weekends" onclick="checkWeekends()"/><br/>
      <input type="button" value="Everyday" onclick="checkEveryday()"/><br/>
      <input type="button" value="Never" onclick="uncheckEveryday()"/><br/>
    </form>
  </body>
</html>
)=====";

String Configurator::pageMiddle PROGMEM = R"=====(

      <label for="name">Name</label>
      <input type="text" name="name" maxlength="20" required/><br/>
      <label for="hour">Time</label>
      <select name="hour">
        <option>00</option>
        <option>01</option>
        <option>02</option>
        <option>03</option>
        <option>04</option>
        <option>05</option>
        <option>06</option>
        <option>07</option>
        <option>08</option>
        <option>09</option>
        <option>10</option>
        <option>11</option>
        <option>12</option>
        <option>13</option>
        <option>14</option>
        <option>15</option>
        <option>16</option>
        <option>17</option>
        <option>18</option>
        <option>19</option>
        <option>20</option>
        <option>21</option>
        <option>22</option>
        <option>23</option>
      </select>
      <label for="minute">:</label>
      <select name="minute"/>
        <option>00</option>
        <option>05</option>
        <option>10</option>
        <option>15</option>
        <option>20</option>
        <option>25</option>
        <option>30</option>
        <option>35</option>
        <option>40</option>
        <option>45</option>
        <option>50</option>
        <option>55</option>
      </select><br/>
      <input type="checkbox" id="sunday" name="sunday"/>
      <label for="sunday">Sunday</label><br/>
      <input type="checkbox" id="monday" name="monday"/>
      <label for="monday">Monday</label><br/>
      <input type="checkbox" id="tuesday" name="tuesday"/>
      <label for="tuesday">Tuesday</label><br/>
      <input type="checkbox" id="wednesday" name="wednesday"/>
      <label for="wednesday">Wednesday</label><br/>
      <input type="checkbox" id="thursday" name="thursday"/>
      <label for="thursday">Thursday</label><br/>
      <input type="checkbox" id="friday" name="friday"/>
      <label for="friday">Friday</label><br/>
      <input type="checkbox" id="saturday" name="saturday"/>
      <label for="saturday">Saturday</label><br/>
      <input type="checkbox" id="skip_phols" name="skip_phols"/>
      <label for="skip_phols">Skip Holidays</label><br/>
      <input type="checkbox" id="once" name="once"/>
      <label for="once">Once</label><br/>
      <input type="checkbox" id="enabled" name="enabled" checked="checked"/>
      <label for="enabled">Enabled</label><br/>
)=====";

String Configurator::editPage(AlarmEntry& alarmEntry) {

  String output = pageTop;
  output.concat("<label for=\"name\">Name</label>\n");
  output.concat("<input type=\"text\" name=\"name\" maxlength=\"20\" value=\"");
  output.concat(alarmEntry.name);
  output.concat("\" readonly=\"readonly\"/><br/>\n");

  // Hour
  output.concat("<label for=\"hour\">Time</label>\n");
  output.concat("<select name=\"hour\">\n");
  for (int x = 0; x < 24; x++) {
    output.concat("<option");
    if (x == alarmEntry.hour) {
      output.concat(" selected");
    }
    output.concat(">");
    if (x < 10) {
      output.concat("0");
    }
    output.concat(x);
    output.concat("</option>\n");
  }
  output.concat("</select>");

  // Minute
  output.concat(":");
  output.concat("<select name=\"minute\">\n");
  for (int x = 0; x < 60; x++) {
    output.concat("<option");
    if (x == alarmEntry.minute) {
      output.concat(" selected");
    }
    output.concat(">");
    if (x < 10) {
      output.concat("0");
    }
    output.concat(x);
    output.concat("</option>\n");
  }
  output.concat("</select><br/>\n");

  output.concat("<input type=\"checkbox\" id=\"sunday\" name=\"sunday\"");
  if (alarmEntry.sunday) {
    output.concat(" checked=\"checked\"");
  }
  output.concat("/><label for=\"sunday\">Sunday</label><br/>");

  output.concat("<input type=\"checkbox\" id=\"monday\" name=\"monday\"");
  if (alarmEntry.monday) {
    output.concat(" checked=\"checked\"");
  }
  output.concat("/><label for=\"monday\">Monday</label><br/>");

  output.concat("<input type=\"checkbox\" id=\"tuesday\" name=\"tuesday\"");
  if (alarmEntry.tuesday) {
    output.concat(" checked=\"checked\"");
  }
  output.concat("/><label for=\"tuesday\">Tuesday</label><br/>");

  output.concat("<input type=\"checkbox\" id=\"wednesday\" name=\"wednesday\"");
  if (alarmEntry.wednesday) {
    output.concat(" checked=\"checked\"");
  }
  output.concat("/><label for=\"wednesday\">Wednesday</label><br/>");

  output.concat("<input type=\"checkbox\" id=\"thursday\" name=\"thursday\"");
  if (alarmEntry.thursday) {
    output.concat(" checked=\"checked\"");
  }
  output.concat("/><label for=\"thursday\">Thursday</label><br/>");

  output.concat("<input type=\"checkbox\" id=\"friday\" name=\"friday\"");
  if (alarmEntry.friday) {
    output.concat(" checked=\"checked\"");
  }
  output.concat("/><label for=\"friday\">Friday</label><br/>");

  output.concat("<input type=\"checkbox\" id=\"saturday\" name=\"saturday\"");
  if (alarmEntry.saturday) {
    output.concat(" checked=\"checked\"");
  }
  output.concat("/><label for=\"saturday\">Saturday</label><br/>");

  output.concat("<input type=\"checkbox\" id=\"skip_phols\" name=\"skip_phols\"");
  if (alarmEntry.skip_phols) {
    output.concat(" checked=\"checked\"");
  }
  output.concat("/><label for=\"skip_phols\">Skip Holidays</label><br/>");

  output.concat("<input type=\"checkbox\" id=\"once\" name=\"once\"");
  if (alarmEntry.once) {
    output.concat(" checked=\"checked\"");
  }
  output.concat("/><label for=\"once\">Once</label><br/>");

  output.concat("<input type=\"checkbox\" id=\"enabled\" name=\"enabled\"");
  if (alarmEntry.enabled) {
    output.concat(" checked=\"checked\"");
  }
  output.concat("/><label for=\"enabled\">Enabled</label><br/>");
  output.concat(pageBottom);

  return output;
}

String Configurator::newPage() {
  String output = pageTop;
  output.concat(pageMiddle);
  output.concat(pageBottom);
  return output;
}
