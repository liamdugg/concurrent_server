window.onload = function() {
    
	console.log("LOADED");

	const host_ip = location.host;

    let updateInterval = 1000; // ms
    let numberElements = 60;
    let updateCount = 0;
    
	let tempChart = document.getElementById('tempChart');
	let pressureChart = document.getElementById('pressureChart');

    let tempOptions = {
		
		scales: {
            
            y: {
				max: 50,
				min: 0,
			},
		},

		plugins: {
			
			legend: {
				display: false
			},
		},

		responsive: true,
		maintainAspectRatio: false,
    };

	let pressureOptions = {
		
		scales: {

            y: { min: 0, },
		},

		plugins: {
			
			legend: {
				display: false
			},
		},

		responsive: true,
		maintainAspectRatio: false,
    };

    let tempChartInstance = new Chart(tempChart, {

		type: 'line',        
		
		data:{
        	datasets: [{
            	label: "Temp [°C]",
            	data: 0,
            	fill: false,
            	borderWidth: 1,
          	}],
        },

        options: tempOptions,
    });

	let pressureChartInstance = new Chart(pressureChart, {

		type: 'line',        
		data:{

        	datasets: [{
            	label: "Presion [hPa]",
            	data: 0,
            	fill: false,
            	borderWidth: 1,
          	}],
        },

        options: pressureOptions,
    });

	async function requestPage() {
		
		let parsedData;
	  
		let dataPromise = async function () {
		  const response = await fetch(host_ip + '/data.csv');
		  const data = await response.text();
		  return data;
		};
	  
		parsedData = await dataPromise();
		
		parsedData = parsedData.split(',');
		parsedData[1] = parseFloat(parsedData[1]);
		parsedData[2] = parseFloat(parsedData[2]);
		
		updateData(parsedData);
		setTimeout(requestPage,updateInterval);
	}

	function addData(data){
      
		if(data){
			
			//console.log(data);
			const temp = document.getElementById('tempDisplay');
			const time = document.getElementById('timeDisplay');
			const pressure = document.getElementById('pressureDisplay');
			
			tempChartInstance.data.labels.push(data[0]);
			tempChartInstance.data.datasets.forEach((dataset) =>{dataset.data.push(data[1])});

			pressureChartInstance.data.labels.push(data[0]);
			pressureChartInstance.data.datasets.forEach((dataset) =>{dataset.data.push(data[2])});
	   
			if(updateCount > numberElements){
			  tempChartInstance.data.labels.shift();
			  tempChartInstance.data.datasets[0].data.shift();
			  pressureChartInstance.data.labels.shift();
			  pressureChartInstance.data.datasets[0].data.shift();
			}
			
			else updateCount++;

			temp.innerHTML = 'Temperatura: ' + String(data[1]) + ' °C';
			pressure.innerHTML = 'Presion: ' + String(data[2]) + ' hPa';
			//console.log(data[0]);
			//console.log(typeof(data[0]));
			time.innerHTML = String(data[0]);

			tempChartInstance.update();
			pressureChartInstance.update();
		  }
	};
	
	function updateData(parsedData){
		addData(parsedData);
	}

	requestPage();
}

