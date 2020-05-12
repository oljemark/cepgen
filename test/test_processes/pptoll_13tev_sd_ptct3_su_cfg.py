import Config.Core as cepgen

process = cepgen.Module('pptoff',
    processParameters = cepgen.Parameters(
        mode = cepgen.ProcessMode.InelasticElastic,
        pair = 13,
    ),
    inKinematics = cepgen.Parameters(
        cmEnergy = 13.e3,
        ktFluxes = 1,
        structureFunctions = cepgen.StructureFunctions.SzczurekUleshchenko,
    ),
    outKinematics = cepgen.Parameters(
        pt = (3.,),
        eta = (-2.5, 2.5),
        mx = (1.07, 1000.),
    )
)

