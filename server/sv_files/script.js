let counter = 0;


function countUp(){
	
	counter++;
	let element_counter = document.getElementById("counter");
	element_counter.innerHTML = counter;

	// wait 3s
	setTimeout(countUp, 500);
}

function changeColor(color){
	let element_counter = document.getElementById("counter");
	element_counter.style.color = color;
}

async function requestPage(){

	const response = await fetch("http://localhost:8989/home.html");
	const ans = await response.text();
	console.log(ans);
	setTimeout(requestPage, 5000);
}