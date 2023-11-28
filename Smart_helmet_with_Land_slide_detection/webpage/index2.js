// Your web app's Firebase configuration
const firebaseConfig = {
    apiKey: "AIzaSyCajH9hHGRxcfjwpqZkfDzReNrI7bdrt74",
    authDomain: "authenticationdb-813af.firebaseapp.com",
    databaseURL: "https://authenticationdb-813af-default-rtdb.asia-southeast1.firebasedatabase.app",
    projectId: "authenticationdb-813af",
    storageBucket: "authenticationdb-813af.appspot.com",
    messagingSenderId: "186023540203",
    appId: "1:186023540203:web:1259691061aa020d6c95ae"
};

// Initialize Firebase
firebase.initializeApp(firebaseConfig);

//intialze variable 
const auth = firebase.auth();
const database = firebase.database();



function register() {

    //getting variable from 
    var username = document.getElementById('getname').value;
    var password = document.getElementById('password').value;

    if (password < 6) {
        alert('password min six letter');
    }
    else {
        console.log('hu');
    }
    auth.createUserWithEmailAndPassword(username, password)
        .then(() => {
            let user = auth.currentUser

            //add user to data base
            var database_ref = database.ref()

            var user_data = {
                email: username,
                password: password
            }
            let add = database_ref.child('user/' + user.uid).set(user_data);
            console.log(add);
        })
        .catch((error) => {
            var error_message = error.message;
            alert(error_message);
        })



}
function userlogin() {
    //getting variable from 
    var username = document.getElementById('getname').value;
    var password = document.getElementById('password').value;

    auth.signInWithEmailAndPassword(username, password)
        .then(() => {
            if (password == 0) {
                return;
            }
            else {
                window.location.href = '/index1.html';
            }
        })
        .catch((error) => {
            var error_message = error.message;
            alert(error_message);
        })

}
