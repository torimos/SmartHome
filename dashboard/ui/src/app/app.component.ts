import { Component, OnInit } from '@angular/core';
import { Chart } from 'chart.js';
import { HttpClient } from '@angular/common/http';
import { Data } from './Data';

@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.sass']
})
export class AppComponent implements OnInit {
  title = 'Smart-Hub Dashboard';
  data: Data[];
  labels = [];
  dataset_t = [];
  dataset_h = [];
  datasetName:String = "";
  chart = [];
  recordsCount = 0;
  latestData: any = {};
  url = 'http://smarthub.local:8080/api/events?limit=288';
  constructor(private httpClient: HttpClient) {}
  ngOnInit() {
    this.httpClient.get(this.url).subscribe((res: Data[]) => {
      res.forEach(y => {
        var ts = new Date(y.createdAt);
        this.labels.push(`${ts.getMonth()}.${ts.getDate()} ${ts.getHours()}:${ts.getMinutes()}:${ts.getSeconds()}`);
        this.dataset_t.push(y.t);
        this.dataset_h.push(y.h);
        this.datasetName = y.dsn;
        this.latestData.t = y.t;
        this.latestData.h = y.h;
        this.latestData.ts = ts;
      });
      this.recordsCount = res.length;

      this.chart = new Chart('canvas', {
        type: 'line',
        data: {
          labels: this.labels,
          datasets: [
            {
              label: "Temperature",
              data: this.dataset_t,
              borderColor: '#FF0000',
              backgroundColor: '#FF0000',
              fill: false,
            },
            {
              label: "Humidity",
              data: this.dataset_h,
              borderColor: '#0000FF',
              backgroundColor: '#0000FF',
              fill: false,
            }
          ]
        },
        options: {
          responsive: true,
					hoverMode: 'index',
					stacked: false,
          legend: {
            display: true
          },
          scales: {
            xAxes: [{
              display: true
            }],
            yAxes: [{
              display: true
            }],
          }
        }
      });
    });
  }
}
