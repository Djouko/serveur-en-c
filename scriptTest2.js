var fs = require("fs");
var form_Script_preview = "";
var send_data = "";
var cpt = 0;
let htmldata = `HTTP/1.1 200 OK
Content-Type: text/html; charset=utf-8
Content-Length: 100000

  <html lang="en">
  <head>
  <meta charset="UTF-8">
  <meta http-equiv="X-UA-Compatible" content="IE=edge">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Document</title>
      <style>
          .app{
              margin-top: 50px;
              margin-left: 20px;
              margin-right: 20px;
              width: 1300px;
              height: 900px;
              display: flex;
              background-color: aquamarine;
              justify-content: space-around;
              align-items: center;
          }
          h1,h5{
              padding-left: 200px;
          }
          .submit{
              margin-left: 250px;
              width: 80px;
              height: 50px;
              border-style: none;
              color: blueviolet;
              background-color: thistle;
          }
  
          #btn{
              height: 40px;
              width: 100px;
              border-style: none;
              color: blueviolet;
              background-color: thistle;
              
          }
      </style>
  </head>
  <body>
      
          <form id="form1" method="post" action="#">
    {{data}}
    <center>
    <input id="btn" type="submit" value="send">
    </center>
    
    </form>

    <form id="sendForm" action="#" method="post" style="display: none;">
        <label for="main-input">Input message:</label>
        <input type="text" id="main-input" name="msg">
        <input type="submit" id="btn" value="Send">
    </form>
    <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js"></script>
    <script>
          {{sendData}}
    </script>
  </body>
</html>

`;

fs.readFile("llms1_config_schema-1.json", "utf8", (err, data) => {
  if (err)
    console.log(
      "Erreur de lecture du fichier de configuration du capteur. Verifiez que la configuration est bien presente sous le nom de json.txt"
    );
  else {
    const json = JSON.parse(data);

    if (json.properties === undefined)
      htmldata = `<h1>Le schema Json passe n'a aucune propriete</h1>`;
    else {
      //Parents
      var parents_properties = json.properties;

      var parents_keys = [];
      var generation = "";

      var form_Script = "";
      for (var key in parents_properties) {
        if (parents_properties.hasOwnProperty(key)) {
          //to be safe
          var option_parent = parents_properties[key];
          send_data = `'${key}'`;
          form_Script_preview = "";

          if (option_parent.description === undefined) {
            let result = generateChild(option_parent.properties);
            generation = `${generation} <div class="app"> <div> <h1>${key}</h1><br>  <div id="${key}"> ${result}</div> </div>  </div>`;
            parents_keys.push(key);
          } else {
            let result = generateChild(option_parent.properties);
            generation = `${generation} <div class="app"> <div> <h1>${key}</h1><h5>${option_parent.description}</h5>   <div id="${key}"> ${result}</div> </div> </div>`;
            parents_keys.push(key);
          }

          // for(var dat in parent_child){

          //     form_Script_preview = `${form_Script_preview} const ${dat} = formData.get(${dat});`
          //     send_data = `${send_data} + " " + ${dat}`
          // }
        }
      }
      form_Script = `${form_Script}

                        //const ${key} = document.querySelector('#${key}')
                        //const btn${key} = document.querySelector('#btn${key}')
                        const btn = document.querySelector('#btn')
                        //const ${key}Submit = document.querySelector('#${key}Submit')
                       /* ${key}Submit.addEventListener('click',${key}Func);
                        function ${key}Func(e){
                            //e.preventDefault();
                            let formData = new FormData(${key});
                            ${form_Script_preview}
                            let url = "http://127.0.0.1:5500/"
                            $.post(url,${send_data}, (data,status) => {
                                console.log(data)
                            })
                                */
                            //console.log(${send_data})
                            //document.querySelector('#main-input${key}').value = ${send_data}

                            //const sendForm = document.querySelector('#sendForm')
                            //const formData2 = new FormData(sendForm);
                            //btn${key}.submit();
                            btn.submit();
                           
                        
                    `;
      console.log(parents_keys);
    }

    htmldata = htmldata.split("{{data}}").join(generation);
    htmldata = htmldata.split("{{sendData}}").join(form_Script);
    //console.log(generation)

    // appendFile function with filename, content and callback function
    fs.writeFile("index.html", htmldata, function (err) {
      if (err) throw err;
      //data = data.replace('{{dollard}}','${')
      console.log("File is created successfully.");
    });
  }
});

function generateChild(child_Properties) {
  var data_child = "";
  for (var key_child in child_Properties) {
    if (child_Properties.hasOwnProperty(key_child)) {
      //to be safe
      cpt = cpt + 1;
      data_child = `${data_child} <label for="${key_child}">${key_child} : </label>`;
      let option_child = child_Properties[key_child];
      console.log(key_child);
      form_Script_preview = `${form_Script_preview} const ${key_child}${cpt} = formData.get('${key_child}${cpt}');`;
      send_data = `${send_data} + ' ' + ${key_child}${cpt}`;

      if (option_child.type === undefined || option_child.type === "string") {
        data_child = `${data_child} <input type="text" name="${key_child}${cpt}" id="${key_child}${cpt}"> <br>`;
      } else if (option_child.type === "number") {
        data_child = `${data_child} <input type="number" id="${key_child}${cpt}" name="${key_child}${cpt}" min="${
          option_child.minimum ? option_child.minimum : 0
        }" max="${
          option_child.maximum ? option_child.maximum : 1000000
        }"> <br>`;
      } else if (option_child.type === "boolean") {
        data_child = `${data_child} <input type="radio" id="${key_child}${cpt}" name="${key_child}${cpt}" value="true"> <label for="true">True   </label> <input type="radio" id="${key_child}${cpt}" name="${key_child}${cpt}" value="false"> <label for="false">False   </label> <br>`;
      }

      if (option_child.description !== undefined) {
        data_child = `${data_child} <small> ${option_child.description}  </small><br><br>`;
      }
      if (option_child.properties !== undefined) {
        data_child = `${data_child} <br> ${generateChild(
          option_child.properties
        )}`;
      }
    }
  }
  return data_child;
}
