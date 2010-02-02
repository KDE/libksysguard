function readSmapsFile() {
    if( !fileExists("/proc/" + process.pid + "/smaps" ) ) {
        if( fileExists("/proc/" + process.pid ) ) {  //Check that it's not just a timing issue - the process might have already ended
            setHtml("<h1>Sorry</h1>Your system is not currently supported (/proc/"+process.pid+"/smaps was not found)");
        }
        return;
    }
    var smaps = readFile("/proc/" + process.pid + "/smaps");
    if(!smaps) {
        if( fileExists("/proc/" + process.pid ) ) {  //Check that it's not just a timing issue - the process might have already ended
            setHtml("<h1>Sorry</h1>Could not read detailed memory information (/proc/"+process.pid+"/smaps could not be read)");
        }
        return;
    }
    return smaps.split('\n');
}

function parseSmaps() {
    var smaps = readSmapsFile();
    if(!smaps)
        return;

    var data = new Array();  /* This is a 0 indexed array */
    /* data contains many dataBlocks */
    var dataBlock; /* This is a hash table */

    var headingRegex = /^([0-9A-Fa-f]+-[0-9A-Fa-f]+) +([^ ]*) +([0-9A-Fa-f]+) +(\d+:\d+) +(\d+) +(.*)$/;
    var lineRegex = /^([^ ]+): +(\d+) kB$/;
    for(var i = 0; i < smaps.length; i++) {
        lineMatch = lineRegex.exec(smaps[i]);
        if(lineMatch) {
            dataBlock[ lineMatch[1] ] = parseInt(lineMatch[2]);  /* E.g.  dataBlock['Size'] = 84 */
            /* Size - Virtual memory space (useless) */
            /* RSS - Includes video card memory etc */
            /* PSS - */
            /* Shared + Private = RSS, but it's more accurate to instead make Shared = Pss - Private */
        } else if(headingMatch = headingRegex.exec(smaps[i])) {
            if(dataBlock)
                data.push(dataBlock);
            dataBlock = new Array();
            dataBlock['address'] = headingMatch[1]; /* Address in address space in the process that it occupies */
            dataBlock['perms'] = headingMatch[2];   /* Read, Write, eXecute, Shared, Private (copy on write) */
            dataBlock['offset'] = headingMatch[3];  /* Offset into the device */
            dataBlock['dev'] = headingMatch[4];     /* Device (major,minor) */
            dataBlock['inode'] = headingMatch[5];   /* inode on the device - 0 means no inode for the memory region - e.g bss */
            dataBlock['pathname'] = headingMatch[6];
        } else if(smaps[i] != "") {
            throw("Could not parse '" + smaps[i]) + "'";
        }
    }
    if(dataBlock)
        data.push(dataBlock);
    // Add in totals
    for(var i = 0; i < data.length; i++) {
        data[i]['Private'] = data[i]['Private_Clean'] + data[i]['Private_Dirty'];
        data[i]['Shared'] = data[i]['Shared_Clean'] + data[i]['Shared_Dirty'];
    }
    return data;
}
function calculateTotal(data, info) {
    var total = 0;
    for(var i = 0; i < data.length; i++) {
        total += data[i][info];
    }
    return total;
}
function formatKB(kb) {
    var format = "";
    if(kb < 2048) /* less than 2MB, write as just KB */
        format = kb + " KB";
    else if(kb < (1048576)) /* less than 1GB, write in MB */
        format = Math.round(kb/1024) + " MB";
    else
        format = Math.round(kb/1048576) + " GB";
    return "<span class=\"memory\">" + format + "</span>";
}
function sortDataByInfo(data, info) {
    return data.sort( function(a,b) { return b[info] - a[info]; } );
}
function getHtmlTableForLibrarySummary(data, info, total) {
    var sortedData = sortDataByInfo(data, info);
    var html = "<table class='librarySummary'><thead><tr><th colspan='2'>" + info + "</th><tbody>";
    for(var i = 0; i < sortedData.length && i < 5; i++) {
        var value = sortedData[i][info] ;
        if(value < (total / 1000))
            break; //Do not bother if library usage is less than 0.1% of the total
        html += "<tr><td class='memory'>" + value + " KB</td><td>" + sortedData[i]['pathname'] + " <span class='perms'>(" + sortedData[i]['perms'] + ")</span></td></tr>";
    }
    html += "</tbody></table>";
    return html;
}
function getHtmlSummary(data) {
    var pss = calculateTotal(data,'Pss');
    var rss = calculateTotal(data,'Rss');
    var private_clean = calculateTotal(data,'Private_Clean');
    var private_dirty = calculateTotal(data,'Private_Dirty');
    var private_total = private_clean + private_dirty;
    var shared_clean = calculateTotal(data,'Shared_Clean');
    var shared_dirty = calculateTotal(data,'Shared_Dirty');
    var shared_total = shared_clean + shared_dirty;
    var swap = calculateTotal(data,'Swap');
    var html = "";
    html += "<h2>Summary</h2>";
    html += "The process <b>" + process.name.substr(0,20) + "</b> (with pid " + process.pid + ") is using approximately " + formatKB(pss) + " of memory.<br>";
    html += "It is using " + formatKB(private_total) + " privately, and a further " + formatKB(shared_total) + " that is, or could be, shared with other programs.<br>";
    if(shared_total != 0)
        html += "Dividing up the shared memory between all the processes sharing that memory we get a reduced shared memory usage of " + formatKB(pss - private_total) + ". Adding that to the private usage, we get the above mentioned total memory footprint of " + formatKB(pss) + ".<br>";
    if( swap != 0)
        html += swap + " KB is swapped out to disk, probably due to a low amount of available memory left.";
    html += "<h2>Library usage</h2>";
    html += "<div class='summaryTable'>" + getHtmlTableForLibrarySummary(data, 'Private', private_total) + "</div>";
    html += "<div class='summaryTable'>" + getHtmlTableForLibrarySummary(data, 'Shared', shared_total) + "</div>";

    html += "<div style='clear:both'><h2>Totals</h2><div class='totalTable'>";
    html += "<table>";
    html += "<tbody>";
    html += "<tr><th class='memory'>Private</th><td class='memory'>" + private_total + " KB</td><td class='comment'>(" + private_clean + " KB clean + " + private_dirty + " KB dirty)</td></tr>";
    html += "<tr><th class='memory'>Shared</th><td class='memory'>" + shared_total + " KB</td><td class='comment'>(" + shared_clean + " KB clean + " + shared_dirty + " KB dirty)</td></tr>";
    html += "<tr><th class='memory'>Rss</th><td class='memory'>" + rss + " KB</td><td class='comment'>(= Private + Shared)</td></tr>";
    html += "<tr><th class='memory'>Pss</th><td class='memory'>" + pss + " KB</td><td class='comment'>(= Private + Shared/Number of Processes)</td></tr>";
    html += "<tr><th class='memory'>Swap</th><td class='memory'>" + swap + " KB</td></tr>";
    html += "</tbody>";
    html += "</table></div></div>";
    return html;
}
function getHtmlHeader() {
    var html = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">";
    html += "<html><head><link href='style.css' rel='stylesheet' type='text/css'>";
    html += "<script language='javascript' src='helper.js' language='javascript' type='text/javascript'/>";
    html += "</head><body>";
    return html;
}
function getHtml() {
    var data = parseSmaps();
    if(!data)
        return;
    var html = getHtmlHeader();
    html += "<div id='innertoc' class='innertoc'></div>";
    html += "<h1>Process " + process.pid + " - " + process.name + "</h1>";
    html += getHtmlSummary(data);
    html += "<div class='fullDetails'><h2>Full details</h2>";
    html += "<table cellspace='2'>";
    html += "<tr><th>Address</th><th>Perm</th><th>Size</th><th align='left'>Filename</th></tr>"
    for(var i = 0; i < data.length; i++) {
        html += "<tr><td class='address'>" + data[i]['address'] + "</td><td class='perms'>" + data[i]['perms'] + "</td><td align='right'>" + data[i]['Size'] + " KB</td><td>" + data[i]['pathname'] + "</td></tr>";
    }
    html += "</table></div>";
    html += "<script language='javascript'>createTOC()</script>";
    html += "</body></html>";
    return html;
}
var result = getHtml();
if(result)
    setHtml(result);

