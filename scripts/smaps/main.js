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
    return "<font color='blue'>" + format + "</font>";
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
    html += "It is using " + formatKB(private_total) + " privately, and a further " + formatKB(shared_total) + " is, or could be, shared with other programs.<br>";
    if(shared_total != 0)
        html += "Dividing up the shared memory between all the processes sharing that memory we get a reduced shared memory usage of " + formatKB(pss - private_total) + ". Adding that to the private usage, we get the above mentioned total memory footprint of " + formatKB(pss) + ".<br>";
    if( swap != 0)
        html += swap + " KB is swapped out to disk, probably due to a low amount of available memory left.";
    html += "<h2>Totals</h2>";
    html += "<table>";
    html += "<tbody>";
    html += "<tr><th align='right'>Private</th><td align='right'>" + private_total + " KB</td><td><font color='gray'>(" + private_clean + " KB clean + " + private_dirty + " KB dirty)</font></td></tr>";
    html += "<tr><th align='right'>Shared</th><td align='right'>" + shared_total + " KB</td><td><font color='gray'>(" + shared_clean + " KB clean + " + shared_dirty + " KB dirty)</font></td></tr>";
    html += "<tr><th align='right'>Private + Shared (Rss)</th><td align='right'>" + rss + " KB</td></tr>";
    html += "<tr><th align='right'>Private + Shared/Number of Processes (Pss)</th><td align='right'>" + pss + " KB</td></tr>";
    html += "<tr><th align='right'>Swaped out to disk</th><td align='right'>" + swap + " KB</td></tr>";
    html += "</tbody>";
    html += "</table>";
    return html;
}
function getHtml() {
    var data = parseSmaps();
    if(!data)
        return;

    var html = "<h1>Process " + process.pid + " - " + process.name + "</h1>";
    html += getHtmlSummary(data);
    html += "<h2>Full details</h2>";
    html += "<table cellspace='2'>";
    html += "<tr><th>Address</th><th>Perm</th><th>Size</th><th align='left'>Filename</th></tr>"
    for(var i = 0; i < data.length; i++) {
        html += "<tr><td nowrap><font color='gray'>" + data[i]['address'] + "</font></td><td nowrap><code>" + data[i]['perms'] + "</code></td><td align='right' nowrap>" + data[i]['Size'] + " KB</td><td nowrap>" + data[i]['pathname'] + "</td></tr>";
    }
    html += "</table>";
    return html;
}
var result = getHtml();
if(result)
    setHtml(result);

