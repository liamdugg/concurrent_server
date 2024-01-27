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
            
			x: {
				
            },

            y: {
				max: 100,
				min: 0,
            },
		},

		plugins: {
			
			title: {
				text: 'Temperatura',
				display: true,
				align: 'center',
				font: {
					size: 20,
				}
			},

			legend: {
				display: false
			},
		},

		responsive: true,
		maintainAspectRatio: false,
    };

	let pressureOptions = {
		
		scales: {
            
			x: {
				
            },

            y: {
				max: 100,
				min: 0,
            },
		},

		plugins: {
			
			title: {
				text: 'Presion',
				display: true,
				align: 'center',
				font: {
					size: 20,
				}
			},

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
            	label: "Presion [kPa]",
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
		parsedData[1] = parseInt(parsedData[1]%100, 10);
		parsedData[2] = parseInt(parsedData[2]%100, 10);
		
		updateData(parsedData);
		setTimeout(requestPage,updateInterval);
	}

	function addData(data){
      
		if(data){
			
			console.log(data);

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
			tempChartInstance.update();
			pressureChartInstance.update();
		  }
	};
	
	function updateData(parsedData){
		console.log(parsedData);
		addData(parsedData);
	}

	requestPage();
}

