function Home() {
  return (
    <section className="mx-auto flex max-w-7xl flex-col py-8 md:pb-12 md:pt-4">
      <div className="max-w-xl">
        <h1 className="text-4xl font-bold leading-tight tracking-tight text-base-content sm:text-5xl lg:text-6xl">
          Welcome to our webserver
        </h1>
      </div>

      <figure className="mt-16 w-full max-w-xl md:mt-24">
        <div className="overflow-hidden rounded-2xl bg-base-200 shadow-lg">
          <img
            className="aspect-4/3 w-full object-cover"
            src={`${import.meta.env.BASE_URL}server-racks.jpg`}
            alt="Rows of server racks in a data center"
            width="1280"
            height="850"
          />
        </div>
      </figure>
    </section>
  );
}

export default Home;
