function setCookie(name, value, daysToExpire) {
    var expirationDate = new Date();
    expirationDate.setDate(expirationDate.getDate() + daysToExpire);
    var expires = "expires=" + expirationDate.toUTCString();
    document.cookie = name + "=" + value + "; " + expires + "; path=/";
}
function setCookieNeverExpires(name, value) {
    var expirationDate = "Fri, 01 Jan 3000 00:00:00 UTC";
    document.cookie = name + "=" + value + ";expires=" + expirationDate + ";path=/";
}

function getCookie(cname) {
    var name = cname + "=";
    var decodedCookie = decodeURIComponent(document.cookie);
    var ca = decodedCookie.split(';');
    for(var i = 0; i < ca.length; i++) {
        var c = ca[i];
        while (c.charAt(0) == ' ') {
            c = c.substring(1);
        }
        if (c.indexOf(name) == 0) {
            return c.substring(name.length, c.length);
        }
    }
    return "";
}
function getlocalstoragemax(){
    if ('storage' in navigator && 'estimate' in navigator.storage) {
        navigator.storage.estimate().then(function(estimate) {
            var maxSizeKB = estimate.quota / 1024;
            console.log("Maximum size of localStorage: " + maxSizeKB + " KB");
        });
    } else {
        console.log("StorageManager API not supported");
    }
}
function getLocalStorageSize() {
    var totalSize = 0;

    for (var key in localStorage) {
        if (localStorage.hasOwnProperty(key)) {
            totalSize += (key.length + localStorage.getItem(key).length) * 2; // length in bytes
        }
    }

    // Convert bytes to kilobytes
    var totalSizeKB = totalSize / 1024;
    
    return totalSizeKB;
}