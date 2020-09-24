const send = function () {
    document.getElementById("submit").disabled=true;
    document.getElementById("success").classList.add("hidden");
    document.getElementById("error").classList.add("hidden");
    document.getElementById("sending").classList.remove("hidden");
    let number = document.getElementById("number").value;
    let text = document.getElementById("text").value;
    let sms = { telephone: number, message: text };
    var httpRequest;
    if (window.XMLHttpRequest) {
        //El explorador implementa la interfaz de forma nativa
        httpRequest = new XMLHttpRequest();
    }
    else if (window.ActiveXObject) {
        //El explorador permite crear objetos ActiveX
        try {
            httpRequest = new ActiveXObject("MSXML2.XMLHTTP");
        } catch (e) {
            try {
                httpRequest = new ActiveXObject("Microsoft.XMLHTTP");
            } catch (e) { }
        }
    }
    if (!httpRequest) {
        alert("No ha sido posible crear una instancia de XMLHttpRequest");
    }
    httpRequest.open("POST", "api/message");
    httpRequest.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
    httpRequest.send(JSON.stringify(sms));

    httpRequest.onload=()=>{
        document.getElementById("sending").classList.add("hidden");
        document.getElementById("success").classList.remove("hidden");
        document.getElementById("submit").disabled=false;
        document.getElementById("number").value="";
        document.getElementById("text").value="";
    };

    httpRequest.onerror=()=>{
        document.getElementById("sending").classList.add("hidden");
        document.getElementById("error").classList.remove("hidden");
        document.getElementById("submit").disabled=false;
    };

    return false;
}