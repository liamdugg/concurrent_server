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


	// fetch es un objeto "promise", devuelve un objeto "response"
	const response = await fetch("http://localhost:8989/home.html");

	console.log(response.status);

	if (response.status == 200){
		const ans = await response.text();
		console.log(ans);
	}

	else {
		console.error('could not fetch data');	
	}

	setTimeout(requestPage, 5000);
}