let counter = 0;

async function requestPage(){

	// fetch es un objeto "promise", devuelve un objeto "response"
	const response = await fetch("http://localhost:8989/data.csv");
	const data = await response.text();

	console.log('Response status --> ' + response.status);

	if (response.status == 200){
		
		const parsed_data = data.split(',');
		let element_counter = document.getElementById("counter");

		element_counter.innerHTML = "<pre>" + parsed_data[0] + "\n" + parsed_data[1] + "</pre>";
		console.log(data);
	}

	else {
		console.error('could not fetch data');	
	}

	//setTimeout(requestPage, 10000);
}