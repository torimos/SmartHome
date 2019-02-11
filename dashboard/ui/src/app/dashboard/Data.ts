// Data.ts

export interface Data {
    dsn: String;
    createdAt: Date;
    t: number;
    h: number;
    p: number;
    v: number;
  }


  export interface QueryResponse {
    items: Data[];
    totalCount: number;
  }