import { Component, OnInit } from '@angular/core';
import { Chart } from 'chart.js';
import { HttpClient } from '@angular/common/http';
import { Data, QueryResponse } from './Data';
import { Options, ChangeContext, PointerType } from 'ng5-slider';

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
  dataset_v = [];
  datasetName:String = "";
  chart: Chart = {};
  latestData: any = {};
  //url = 'http://localhost:8080/api/events';
  url = 'http://smarthub.local:8080/api/events';
  sliderSteps: number = 24*60/ 30; // 24*60 / interval
  sliderValue = 0;
  oldSliderValue = -1;
  sliderOptions: Options = null;
  constructor(private httpClient: HttpClient) {}
  onSliderChange(changeContext: ChangeContext): void {
    if (changeContext.value != this.oldSliderValue) {
      this.refreshData();
      this.oldSliderValue = changeContext.value;
    }
  };
  refreshData() {
    this.data = [];
    this.labels = [];
    this.dataset_t = [];
    this.dataset_h = [];
    this.dataset_v = [];
    this.latestData = {};
    this.httpClient.get(this.url + `?limit=${this.sliderSteps}&skip=${this.sliderValue*this.sliderSteps}`).subscribe((res: QueryResponse) => {
      let idx = 0;
      res.items.forEach(y => {
        //if ((idx%12)==0) 
        {
          var ts = new Date(y.createdAt);
          var v = (Number)(y.v)*100.0/4096;
          this.labels.push(`${ts.getMonth()+1}.${ts.getDate()} ${ts.getHours()}:${ts.getMinutes()}:${ts.getSeconds()}`);
          this.dataset_t.push(y.t);
          this.dataset_h.push(y.h);
          this.dataset_v.push(v);
          this.datasetName = y.dsn;
          this.latestData.t = y.t;
          this.latestData.h = y.h;
          this.latestData.v = v;
          this.latestData.ts = ts;
        }
        idx++;
      });

      let totalCount = this.sliderSteps * 7;

      this.sliderOptions = {
        floor: 0,
        ceil: Math.floor(totalCount/this.sliderSteps),
        step: 1,
        showTicks: true,
        showTicksValues: true
      };
      if (this.chart.destroy != null) 
      {
        this.chart.destroy();
      }
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
            },
            {
              label: "Battery",
              data: this.dataset_v,
              borderColor: '#003F00',
              backgroundColor: '#003F00',
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
  };
  ngOnInit() {
    
    this.refreshData();
  };
}
